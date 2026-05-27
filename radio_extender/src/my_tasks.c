#include "tx_api.h"
#include "uart.h"
#include "cc1101.h"
#include "protocol.h"
#include "cobs.h"
#include "bat_brd.h"
#include "printk.h"
#include "my_tasks.h"
#include "usb_thr.h"
#include "system_at32f415.h"
#include <stddef.h>
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
#define RF_COBS_ENC_MAX         COBS_ENCODED_MAX(PROTO_MAX_PACKET)
/* COBS(5-байт PING/PONG) = 6; меньше — не наш кадр */
#define RF_COBS_WIRE_MIN        6U

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
static uint8_t pong_ok_streak = 0;
static uint8_t link_liveness = 0;
static volatile uint8_t link_refresh_tick = 0;
static volatile uint8_t cycle_got_expected_pong = 0;
static volatile uint8_t rf_payload_pending = 0;
static volatile uint8_t rf_payload_waiting_ack = 0;
static volatile ULONG rf_payload_ack_deadline = 0;
static uint8_t hb_ping_outstanding = 0;

static uint8_t host_cobs_acc[RF_COBS_ENC_MAX];
static cobs_decoder_t host_cobs_dec;

#define RF_LINK_CHECK_MS          100U
#define RF_LINK_MISS_LIMIT        3U
#define RF_PING_DEDUPE_MS         800U
#define RF_OWN_PING_ECHO_MS       25U
#define RF_PONG_OK_CYCLES         3U
#define RF_PAYLOAD_ACK_MS         100U

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
    uint8_t wire_buf[RF_COBS_ENC_MAX + 1U];
    uint8_t fifo_buf[RF_COBS_ENC_MAX + 2U];
    size_t enc;
    uint8_t wire_len;

    if (pkt == NULL || len < RF_LINK_PKT_LEN || len > PROTO_MAX_PACKET ||
        pkt[0] + 2u != len) {
        return;
    }

    enc = cobs_encode(pkt, len, wire_buf);
    if (enc == 0U || enc >= RF_COBS_ENC_MAX) {
        return;
    }
    /* На RF длину задаёт CC1101; 0x00-разделитель только для UART */
    wire_len = (uint8_t)enc;

    cc1101_lock();
    CC1101_SendCmd(CC1101_CMD_SIDLE);
    rf_busy_wait_us(150);
    CC1101_SendCmd(CC1101_CMD_SFTX);
    CC1101_ApplyVariablePacketMode();
    fifo_buf[0] = wire_len;
    memcpy(&fifo_buf[1], wire_buf, wire_len);
    CC1101_WriteBurst(CC1101_REG_FIFO, fifo_buf, (uint8_t)(wire_len + 1U));
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
    }
    missed_pings = 0;
}

static void link_on_traffic(void)
{
    cycle_got_expected_pong = 1;
    rf_payload_pending = 0;
    rf_payload_waiting_ack = 0;
    rf_payload_ack_deadline = 0;
    hb_ping_outstanding = 0;
    missed_pings = 0;
}

static uint8_t rf_tx_queue_has_messages(void)
{
    ULONG enqueued = 0;
    ULONG available = 0;
    ULONG suspended_count = 0;
    TX_THREAD *first_suspended = TX_NULL;
    TX_QUEUE *next_queue = TX_NULL;
    CHAR *name = TX_NULL;

    if (tx_queue_info_get(&q_rf_tx, &name, &enqueued, &available,
                          &first_suspended, &suspended_count, &next_queue) != TX_SUCCESS) {
        return 0;
    }
    return (enqueued > 0U) ? 1U : 0U;
}

static void bridge_send_to_host(const uint8_t *pkt, uint8_t pkt_len)
{
    uint8_t wire[RF_COBS_ENC_MAX + 1U];
    size_t enc;

    if (pkt == NULL || pkt_len < RF_LINK_PKT_LEN) {
        return;
    }
    enc = cobs_encode(pkt, pkt_len, wire);
    if (enc == 0U || enc >= RF_COBS_ENC_MAX) {
        return;
    }
    wire[enc++] = 0;
    (void)cdc_send_frame(wire, (uint16_t)enc);
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
        printk("\n\r RF pkt reject plen=%u LEN=%u need=%u",
               (unsigned)pkt_len,
               (unsigned)rf_receive_pkt[0],
               (unsigned)(rf_receive_pkt[0] + 2u));
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
                (void)rf_tx_enqueue(resp, rlen);
            }
        } else if (type == PROTO_TYPE_PONG) {
            if (pkt_len != RF_LINK_PKT_LEN) {
                return;
            }
            if (id != expect_pong_id) {
                return;
            }
            /* id общий у PING/PONG: не отбрасывать ожидаемый PONG из-за своего PONG с тем же id */
            link_on_traffic();
        } else if (type == PROTO_TYPE_DATA) {
            if (pkt_len <= RF_LINK_PKT_LEN) {
                return;
            }
            link_on_traffic();
            UART_PC_WriteBuffer(rf_receive_data, len);
            bridge_send_to_host(rf_receive_pkt, pkt_len);
        }
    }
}

/* COBS-блок из FIFO CC1101 (без 0x00 на хвосте). 0 = ошибка. */
static uint8_t rf_wire_decode(const uint8_t *wire, uint8_t wire_len,
                              uint8_t *pkt, uint8_t *pkt_len_out)
{
    size_t raw_len;
    size_t cobs_len;

    if (wire == NULL || pkt == NULL || pkt_len_out == NULL || wire_len < 2U) {
        return 0;
    }

    cobs_len = wire_len;
    if (wire[wire_len - 1U] == 0U) {
        cobs_len = (size_t)(wire_len - 1U);
    }

    raw_len = cobs_decode(wire, cobs_len, pkt);
    if (raw_len < RF_LINK_PKT_LEN || raw_len > PROTO_MAX_PACKET) {
        return 0;
    }
    *pkt_len_out = (uint8_t)raw_len;
    return 1;
}

static void host_cobs_frame_cb(const uint8_t *frame, size_t len, void *ctx)
{
    uint8_t id_p;
    uint8_t type_p;
    uint8_t out_len;

    (void)ctx;

    if (len > PROTO_MAX_PACKET || len < RF_LINK_PKT_LEN) {
        return;
    }
    memcpy(uart_rx_buf, frame, len);
    if (!Protocol_ParsePacket(uart_rx_buf, (uint8_t)len, &id_p, &type_p, uart_rx_data, &out_len)) {
        return;
    }
    if (type_p != PROTO_TYPE_DATA) {
        return;
    }
    rf_payload_pending = 1;
    if (tx_queue_send(&q_rf_tx, uart_rx_buf, TX_NO_WAIT) != TX_SUCCESS) {
        printk("\n\r RF tx queue full (host)");
    }
}

void bridge_host_rx_bytes(const uint8_t *data, size_t len)
{
    if (data != NULL && len > 0U) {
        cobs_decoder_feed(&host_cobs_dec, data, len);
    }
}

/* Сырой кадр [LEN][ID][TYPE][CRC][CRC] в FIFO (CC1101 len=5). */
static uint8_t rf_try_raw_link_frame(const uint8_t *wire, uint8_t wire_len,
                                     uint8_t *pkt_len_out)
{
    if (wire == NULL || pkt_len_out == NULL || wire_len != RF_LINK_PKT_LEN) {
        return 0;
    }
    if (wire[0] + 2u != RF_LINK_PKT_LEN) {
        return 0;
    }
    memcpy(rf_receive_pkt, wire, RF_LINK_PKT_LEN);
    *pkt_len_out = RF_LINK_PKT_LEN;
    return 1;
}

/* 1 = обработан хотя бы один кадр. Вызывать без удержания mutex. */
static uint8_t cc1101_poll_and_handle_rx(void)
{
    uint8_t rxb;
    uint8_t n;
    uint8_t plen;
    uint8_t pkt_len;
    uint8_t wire_buf[CC1101_MAX_PKTLEN];
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
        if (n < 2U) {
            break;
        }

        CC1101_ReadBurst(CC1101_REG_FIFO, &plen, 1);
        if (plen > CC1101_MAX_PKTLEN || n < (uint8_t)(plen + 1U)) {
            cc1101_recover_rx(0);
            break;
        }
        /* plen=3 и b0=0x78 — мусор/обрыв COBS; не сбрасывать весь RX */
        if (plen < RF_LINK_PKT_LEN) {
            CC1101_ReadBurst(CC1101_REG_FIFO, wire_buf, plen);
            continue;
        }

        CC1101_ReadBurst(CC1101_REG_FIFO, wire_buf, plen);
        cc1101_unlock();

        handled = 1;
        pkt_len = 0;
        if ((plen == RF_LINK_PKT_LEN && rf_try_raw_link_frame(wire_buf, plen, &pkt_len)) ||
            (plen >= RF_COBS_WIRE_MIN &&
             rf_wire_decode(wire_buf, plen, rf_receive_pkt, &pkt_len))) {
            cc1101_process_rx_frame(pkt_len);
        } else {
            uint8_t id_raw;
            uint8_t type_raw;
            uint8_t data_len_raw;

            if (plen >= RF_LINK_PKT_LEN &&
                Protocol_ParsePacket(wire_buf, plen, &id_raw, &type_raw,
                                     rf_receive_data, &data_len_raw)) {
                memcpy(rf_receive_pkt, wire_buf, plen);
                cc1101_process_rx_frame(plen);
            } else if (plen < RF_COBS_WIRE_MIN) {
                /* короткий блок — пропуск */
            } else {
                printk("\n\r RF decode fail plen=%u b0=%02X",
                       (unsigned)plen, (unsigned)wire_buf[0]);
            }
        }

        cc1101_lock();
    }

    CC1101_StrobeRx();
    cc1101_unlock();

    return handled;
}

void task_uart_rx(ULONG thread_input)
{
    uint8_t b;

    (void)thread_input;

    for (;;) {
        if (UART_PC_Available()) {
            b = UART_PC_ReadByte();
            bridge_host_rx_bytes(&b, 1);
        }
        tx_thread_sleep(1);
    }
}

void task_rf_send(ULONG thread_input)
{
    (void)thread_input;

    for (;;) {
        if (tx_queue_receive(&q_rf_tx, rf_send_pkt, TX_WAIT_FOREVER) == TX_SUCCESS) {
            uint8_t tx_len;

            if (rf_send_pkt[0] < 3U || (uint16_t)rf_send_pkt[0] + 2u > PROTO_MAX_PACKET) {
                continue;
            }
            tx_len = (uint8_t)(rf_send_pkt[0] + 2u);
            cc1101_send_packet(rf_send_pkt, tx_len);
            if (rf_send_pkt[2] == PROTO_TYPE_DATA) {
                rf_payload_pending = 0;
                rf_payload_waiting_ack = 1;
                rf_payload_ack_deadline = tx_time_get() + RF_PAYLOAD_ACK_MS;
            }
        }
    }
}

void task_heartbeat(ULONG thread_input)
{
    (void)thread_input;

    put_led(LED_RED);
    tx_thread_sleep(rf_ping_phase_ms());
    hb_ping_outstanding = 0;

    for (;;) {
        ULONG now;

        tx_thread_sleep(RF_LINK_CHECK_MS);
        now = tx_time_get();

        if (rf_payload_pending || rf_payload_waiting_ack || rf_tx_queue_has_messages()) {
            if (rf_payload_waiting_ack && now >= rf_payload_ack_deadline &&
                !cycle_got_expected_pong) {
                missed_pings++;
                rf_payload_waiting_ack = 0;
                if (missed_pings >= RF_LINK_MISS_LIMIT) {
                    link_mark_lost();
                }
            }
            if (cycle_got_expected_pong) {
                cycle_got_expected_pong = 0;
                missed_pings = 0;
                if (pong_ok_streak < 255U) {
                    pong_ok_streak++;
                }
                if (pong_ok_streak >= RF_PONG_OK_CYCLES) {
                    link_mark_ok();
                } else {
                    link_refresh();
                }
            }
            put_led(g_link_status);
            continue;
        }

        if (hb_ping_outstanding) {
            hb_ping_outstanding = 0;
            if (cycle_got_expected_pong) {
                cycle_got_expected_pong = 0;
                missed_pings = 0;
                if (pong_ok_streak < 255U) {
                    pong_ok_streak++;
                }
                if (pong_ok_streak >= RF_PONG_OK_CYCLES) {
                    link_mark_ok();
                } else {
                    link_refresh();
                }
            } else if (link_liveness > 0U) {
                link_liveness--;
                missed_pings = 0;
                pong_ok_streak = 0;
            } else {
                pong_ok_streak = 0;
                missed_pings++;
                if (missed_pings >= RF_LINK_MISS_LIMIT) {
                    link_mark_lost();
                }
            }
        } else {
            ping_seq++;
            expect_pong_id = ping_seq;
            last_own_ping_tx_id = expect_pong_id;
            last_own_ping_tx_ms = now;
            if (rf_tx_enqueue_link(expect_pong_id, PROTO_TYPE_PING)) {
                hb_ping_outstanding = 1;
            }
        }

        put_led(g_link_status);
    }
}

void task_rf_receive(ULONG thread_input)
{
    (void)thread_input;

    cc1101_lock();
    cc1101_rf_to_rx();
    cc1101_unlock();

    for (;;) {
        uint8_t poll;

        cc1101_lock();
        poll = cc1101_gdo0_pkt_ready() ||
               ((CC1101_ReadReg(CC1101_REG_RXBYTES) & 0x7Fu) > 0U);
        cc1101_unlock();

        if (poll) {
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

    cobs_decoder_init(&host_cobs_dec, host_cobs_acc, RF_COBS_ENC_MAX,
                      host_cobs_frame_cb, NULL);

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

    put_led(LED_RED);
}
