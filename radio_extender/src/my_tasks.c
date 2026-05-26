#include "tx_api.h"
#include "uart.h"
#include "cc1101.h"
#include "protocol.h"
#include "bat_brd.h"
#include "printk.h"
#include "my_tasks.h"
#include "usb_thr.h"
#include "system_at32f415.h"
#include <string.h>

#define RF_TX_QUEUE_DEPTH       8
#define RF_TX_MSG_ULONGS        (PROTO_MAX_PACKET / sizeof(ULONG))

#define PRIORITY_RF_HEARTBEAT   6
#define PRIORITY_RF_SEND        7
#define PRIORITY_RF_RECEIVE     9
#define PRIORITY_UART_RX        10
#define PRIORITY_USB            USB_PRIORITY

#define STACK_RF_SEND           1536
#define STACK_RF_RECEIVE        2048
#define STACK_HEARTBEAT         1024
#define STACK_UART_RX           1536
#define STACK_USB               USB_TASK_STACK_SIZE

#define MCU_UID_BASE            0x1FFFF7E8U
#define RF_LINK_PKT_LEN         5U
#define RF_LINK_LIVE_CYCLES     3U

static TX_THREAD thread_rf_send;
static TX_THREAD thread_rf_receive;
static TX_THREAD thread_heartbeat;
static TX_THREAD thread_uart_rx;
static TX_THREAD thread_usb;
static TX_MUTEX  cc1101_mutex;

static ULONG rf_tx_queue_buffer[RF_TX_QUEUE_DEPTH * RF_TX_MSG_ULONGS];

static UCHAR stack_uart_rx_mem[STACK_UART_RX];
static UCHAR stack_rf_send_mem[STACK_RF_SEND];
static UCHAR stack_rf_receive_mem[STACK_RF_RECEIVE];
static UCHAR stack_heartbeat_mem[STACK_HEARTBEAT];
static UCHAR stack_usb_mem[STACK_USB];

TX_QUEUE q_rf_tx;

static volatile uint8_t g_link_status = LED_RED;
static uint8_t missed_pings = 0;
/* PONG с id последнего PING принимается до следующего PING (весь цикл ~1 с). */
static volatile uint8_t expect_pong_id = 0;
static uint8_t ping_seq = 0;
static volatile ULONG last_own_ping_tx_ms = 0;
static volatile uint8_t last_own_ping_tx_id = 0;
static ULONG last_pong_tx_ms = 0;
static uint8_t last_pong_for_ping_id = 0;
static uint8_t last_peer_ping_id = 0;
static ULONG last_own_pong_tx_ms = 0;
static uint8_t last_own_pong_tx_id = 0;
static uint8_t pong_ok_streak = 0;
static uint8_t link_liveness = 0;
static volatile uint8_t link_refresh_tick = 0;
static volatile uint8_t cycle_got_expected_pong = 0;

#define RF_PING_DEDUPE_MS         800U
#define RF_OWN_PONG_IGNORE_MS     800U
#define RF_OWN_PING_ECHO_MS       25U
#define RF_PONG_OK_CYCLES         3U

/* id считается «новее» prev (учёт переполнения uint8). */
static uint8_t ping_id_is_newer(uint8_t id, uint8_t prev)
{
    return (uint8_t)(id - prev) < 128u;
}

uint8_t uart_rx_buf[PROTO_MAX_PACKET];
uint8_t uart_rx_data[PROTO_MAX_PACKET];
uint8_t rf_send_pkt[PROTO_MAX_PACKET];
uint8_t rf_receive_pkt[PROTO_MAX_PACKET];
uint8_t rf_receive_data[PROTO_MAX_PACKET];

static void rf_busy_wait_us(uint32_t us)
{
    volatile uint32_t i;
    uint32_t loops = (system_core_clock / 1000000U) * us / 4U;

    if (loops == 0U) {
        loops = 1U;
    }
    for (i = 0; i < loops; i++) {
        __NOP();
    }
}

/* Сдвиг PING 100..599 ms */
static ULONG rf_ping_phase_ms(void)
{
    volatile uint32_t *uid = (volatile uint32_t *)MCU_UID_BASE;
    uint32_t mix = uid[0] + uid[1] + uid[2];

    return 100U + (ULONG)(mix % 500U);
}

static void cc1101_lock(void)
{
    tx_mutex_get(&cc1101_mutex, TX_WAIT_FOREVER);
}

static void cc1101_unlock(void)
{
    tx_mutex_put(&cc1101_mutex);
}

static void cc1101_rf_to_rx(void)
{
    CC1101_EnterRx();
}

static void cc1101_recover_rx(uint8_t log_overflow)
{
    static uint8_t ovf_log_count;

    if (log_overflow && ovf_log_count < 4u) {
        ovf_log_count++;
        printk("\n\r RF RX overflow, recover #%u", ovf_log_count);
    }
    CC1101_FlushRxFifo();
    CC1101_StrobeRx();
}

static void cc1101_wait_tx_done(void)
{
    rf_busy_wait_us(8000);
}

static void cc1101_send_packet(const uint8_t *pkt, uint8_t len)
{
    if (pkt == NULL || len < RF_LINK_PKT_LEN || len > PROTO_MAX_PACKET) {
        return;
    }

    cc1101_lock();
    CC1101_SendCmd(CC1101_CMD_SIDLE);
    rf_busy_wait_us(150);
    CC1101_SendCmd(CC1101_CMD_SFRX);
    CC1101_SendCmd(CC1101_CMD_SFTX);
    CC1101_ApplyVariablePacketMode();
    {
        uint8_t fifo_buf[PROTO_MAX_PACKET + 1U];

        fifo_buf[0] = len;
        memcpy(&fifo_buf[1], pkt, len);
        CC1101_WriteBurst(CC1101_REG_FIFO, fifo_buf, (uint8_t)(len + 1U));
    }
    CC1101_SendCmd(CC1101_CMD_STX);
    cc1101_wait_tx_done();
    CC1101_ListenAfterTx();
    cc1101_unlock();
}

static uint8_t cc1101_gdo0_pkt_ready(void)
{
    return get_gdo0();
}

static void link_refresh(void)
{
    link_refresh_tick = 1;
    link_liveness = RF_LINK_LIVE_CYCLES;
}

static uint8_t rf_link_build(uint8_t id, uint8_t type, uint8_t *out)
{
    return Protocol_BuildPacket(id, type, NULL, 0, out);
}

/* В очередь q_rf_tx; передачу делает только task_rf_send. */
static uint8_t rf_tx_enqueue(const uint8_t *pkt, uint8_t pkt_len)
{
    if (pkt == NULL || pkt_len < RF_LINK_PKT_LEN || pkt_len > PROTO_MAX_PACKET ||
        pkt[0] + 2u != pkt_len) {
        return 0;
    }
    if (tx_queue_send(&q_rf_tx, (void *)pkt, TX_NO_WAIT) != TX_SUCCESS) {
        printk("\n\r RF tx queue full");
        return 0;
    }
    return 1;
}

static uint8_t rf_tx_enqueue_link(uint8_t id, uint8_t type)
{
    uint8_t pkt[10];
    uint8_t len = rf_link_build(id, type, pkt);

    if (len == 0U) {
        return 0;
    }
    return rf_tx_enqueue(pkt, len);
}

static void link_mark_ok(void)
{
    link_refresh();
    if (g_link_status != LED_GREEN) {
        g_link_status = LED_GREEN;
        put_led(LED_GREEN);
        printk("\n\r RF link OK");
    }
    missed_pings = 0;
}

static void link_mark_lost(void)
{
    if (g_link_status != LED_RED) {
        printk("\n\r RF link lost");
    }
    g_link_status = LED_RED;
    put_led(LED_RED);
    last_peer_ping_id = 0;
    pong_ok_streak = 0;
    link_liveness = 0;
}

/* Разбор кадра протокола (5…PROTO_MAX_PACKET байт) в rf_receive_pkt. */
static void cc1101_process_rx_frame(uint8_t pkt_len)
{
    uint8_t id;
    uint8_t type;
    uint8_t len;

    if (pkt_len < RF_LINK_PKT_LEN || pkt_len > PROTO_MAX_PACKET ||
        rf_receive_pkt[0] + 2u != pkt_len) {
        return;
    }

    if (Protocol_ParsePacket(rf_receive_pkt, pkt_len, &id, &type, rf_receive_data, &len)) {
        if (type == PROTO_TYPE_PING) {
            if (pkt_len != RF_LINK_PKT_LEN) {
                return;
            }
            ULONG now = tx_time_get();

            if (id == 0U) {
                return;
            }
            /* Только эхо своего PING сразу после TX; id совпадает у соседей — это норма. */
            if (id == last_own_ping_tx_id &&
                (now - last_own_ping_tx_ms) < RF_OWN_PING_ECHO_MS) {
                return;
            }
            if (last_peer_ping_id != 0U && !ping_id_is_newer(id, last_peer_ping_id)) {
                return;
            }
            if (id == last_pong_for_ping_id && (now - last_pong_tx_ms) < RF_PING_DEDUPE_MS) {
                return;
            }
            {
                uint8_t resp[10];
                uint8_t rlen = rf_link_build(id, PROTO_TYPE_PONG, resp);

                last_peer_ping_id = id;
                last_pong_for_ping_id = id;
                last_pong_tx_ms = now;
                last_own_pong_tx_id = id;
                last_own_pong_tx_ms = now;
                if (rf_tx_enqueue(resp, rlen)) {
                    printk("\n\r RF rx PING id=%u -> PONG q", (unsigned)id);
                }
            }
        } else if (type == PROTO_TYPE_PONG) {
            if (pkt_len != RF_LINK_PKT_LEN) {
                return;
            }
            if (id == expect_pong_id) {
                ULONG now = tx_time_get();

                if (id == last_own_pong_tx_id &&
                    (now - last_own_pong_tx_ms) < RF_OWN_PONG_IGNORE_MS) {
                    return;
                }
                cycle_got_expected_pong = 1;
                printk("\n\r RF rx PONG id=%u", (unsigned)id);
            }
        } else if (type == PROTO_TYPE_DATA) {
            if (pkt_len <= RF_LINK_PKT_LEN) {
                return;
            }
            UART_PC_WriteBuffer(rf_receive_data, len);
            cdc_send_frame(rf_receive_pkt, pkt_len);
        }
    }
}

/* 1 = обработан хотя бы один кадр. Вызывать без удержания mutex. */
static uint8_t cc1101_poll_and_handle_rx(void)
{
    uint8_t rxb;
    uint8_t n;
    uint8_t plen;
    uint8_t handled;

    handled = 0;

    cc1101_lock();
    CC1101_ApplyVariablePacketMode();

    for (;;) {
        rxb = CC1101_ReadReg(CC1101_REG_RXBYTES);
        if (rxb & CC1101_RXBYTES_OVERFLOW) {
            cc1101_recover_rx(0);
            break;
        }
        n = rxb & 0x7Fu;
        if (n < (RF_LINK_PKT_LEN + 1U)) {
            break;
        }
        CC1101_ReadBurst(CC1101_REG_FIFO, &plen, 1);
        if (plen < RF_LINK_PKT_LEN || plen > PROTO_MAX_PACKET) {
            cc1101_recover_rx(0);
            break;
        }
        if (n < (uint8_t)(plen + 1U)) {
            break;
        }
        CC1101_ReadBurst(CC1101_REG_FIFO, rf_receive_pkt, plen);
        cc1101_unlock();

        handled = 1;
        cc1101_process_rx_frame(plen);

        cc1101_lock();
    }

    rxb = CC1101_ReadReg(CC1101_REG_RXBYTES);
    if ((rxb & 0x7Fu) > 0) {
        CC1101_SendCmd(CC1101_CMD_SFRX);
    }
    CC1101_StrobeRx();
    cc1101_unlock();

    return handled;
}

void task_uart_rx(ULONG thread_input)
{
    uint8_t idx = 0;

    (void)thread_input;

    for (;;) {
        if (UART_PC_Available()) {
            uint8_t b = UART_PC_ReadByte();
            if (idx == 0) {
                if (b > 0 && b <= PROTO_MAX_PACKET - 5) {
                    uart_rx_buf[idx++] = b;
                }
            } else {
                if (idx >= PROTO_MAX_PACKET) {
                    idx = 0;
                    continue;
                }
                uart_rx_buf[idx++] = b;
                if (idx >= uart_rx_buf[0] + 2) {
                    uint8_t out_len, id_p, type_p;
                    if (Protocol_ParsePacket(uart_rx_buf, idx, &id_p, &type_p, uart_rx_data, &out_len)) {
                        if (type_p == PROTO_TYPE_DATA) {
                            tx_queue_send(&q_rf_tx, uart_rx_buf, TX_NO_WAIT);
                        }
                    }
                    idx = 0;
                }
            }
        }
        tx_thread_sleep(1);
    }
}

void task_rf_send(ULONG thread_input)
{
    (void)thread_input;

    printk("\n\r task_rf_send");
    for (;;) {
        if (tx_queue_receive(&q_rf_tx, rf_send_pkt, TX_WAIT_FOREVER) == TX_SUCCESS) {
            uint8_t n = rf_send_pkt[0];
            uint8_t tx_len;

            if (n == 0U || (uint16_t)n + 2u > PROTO_MAX_PACKET) {
                continue;
            }
            tx_len = (uint8_t)(n + 2u);
            cc1101_send_packet(rf_send_pkt, tx_len);
            printk("\n\r RF tx id=%u type=%02X", (unsigned)rf_send_pkt[1],
                   (unsigned)rf_send_pkt[2]);
        }
    }
}

void task_heartbeat(ULONG thread_input)
{
    (void)thread_input;

    printk("\n\r task_heartbeat");
    put_led(LED_RED);
    tx_thread_sleep(rf_ping_phase_ms());

    for (;;) {
        {
            ping_seq++;
            expect_pong_id = ping_seq;
            link_refresh_tick = 0;
            cycle_got_expected_pong = 0;
            last_own_ping_tx_id = expect_pong_id;
            last_own_ping_tx_ms = tx_time_get();
            if (rf_tx_enqueue_link(expect_pong_id, PROTO_TYPE_PING)) {
                printk("\n\r RF PING q id=%u", (unsigned)expect_pong_id);
            }
            /* Весь приём — в task_rf_receive; здесь только ждём. */
            tx_thread_sleep(1000);

            if (cycle_got_expected_pong) {
                link_refresh_tick = 1;
                link_liveness = RF_LINK_LIVE_CYCLES;
                missed_pings = 0;
                if (pong_ok_streak < 255U) {
                    pong_ok_streak++;
                }
                if (pong_ok_streak >= RF_PONG_OK_CYCLES) {
                    link_mark_ok();
                }
            } else if (link_liveness > 0U) {
                link_liveness--;
                missed_pings = 0;
                pong_ok_streak = 0;
            } else {
                pong_ok_streak = 0;
                missed_pings++;
                if (missed_pings >= 2U) {
                    link_mark_lost();
                }
            }
            put_led(g_link_status);
        }
    }
}

void task_rf_receive(ULONG thread_input)
{
    (void)thread_input;

    printk("\n\r task_rf_receive");
    cc1101_lock();
    cc1101_rf_to_rx();
    cc1101_unlock();

    for (;;) {
        if (cc1101_gdo0_pkt_ready()) {
            (void)cc1101_poll_and_handle_rx();
            tx_thread_sleep(1);
        } else {
            tx_thread_sleep(5);
        }
    }
}

void tasks_init(void)
{
    UINT status;

    status = tx_mutex_create(&cc1101_mutex, "CC1101", TX_NO_INHERIT);
    if (status != TX_SUCCESS) {
        printk("\n\r error cc1101 mutex");
        return;
    }

    cc1101_lock();
    if (!CC1101_ReapplyRadio()) {
        printk("\n\r CC1101 radio cfg FAIL");
    }
    cc1101_unlock();

    printk("\n\r RF ping phase=%lu ms", (unsigned long)rf_ping_phase_ms());

    status = tx_queue_create(&q_rf_tx, "rf_tx", RF_TX_MSG_ULONGS,
                             rf_tx_queue_buffer, sizeof(rf_tx_queue_buffer));
    if (status != TX_SUCCESS) {
        printk("\n\r error rf_tx queue create");
        return;
    }

    status = tx_thread_create(&thread_uart_rx, "UART_RX", task_uart_rx, 0,
                              stack_uart_rx_mem, STACK_UART_RX,
                              PRIORITY_UART_RX, PRIORITY_UART_RX,
                              TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        printk("\n\r error task_uart_rx create");
        return;
    }
    status = tx_thread_create(&thread_usb, "USB", usb_task, 0,
                              stack_usb_mem, STACK_USB,
                              PRIORITY_USB, USB_PRIORITY,
                              TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        printk("\n\r error usb_task create");
        return;
    }

    status = tx_thread_create(&thread_rf_receive, "RF_RECV", task_rf_receive, 0,
                              stack_rf_receive_mem, STACK_RF_RECEIVE,
                              PRIORITY_RF_RECEIVE, PRIORITY_RF_RECEIVE,
                              TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        printk("\n\r error task_rf_receive create");
        return;
    }

    status = tx_thread_create(&thread_rf_send, "RF_SEND", task_rf_send, 0,
                              stack_rf_send_mem, STACK_RF_SEND,
                              PRIORITY_RF_SEND, PRIORITY_RF_SEND,
                              TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        printk("\n\r error task_rf_send create");
        return;
    }

    status = tx_thread_create(&thread_heartbeat, "HB", task_heartbeat, 0,
                              stack_heartbeat_mem, STACK_HEARTBEAT,
                              PRIORITY_RF_HEARTBEAT, PRIORITY_RF_HEARTBEAT,
                              TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        printk("\n\r error task_heartbeat create");
        return;
    }

    printk("\n\r OK task create");
    put_led(LED_RED);
}
