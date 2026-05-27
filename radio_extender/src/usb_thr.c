#include "rtos.h"
#include "usb.h"
#include "usb_thr.h"
#include "usbx_cdc.h"
#include "usbx_app.h"
#include "usbd_usbx_class.h"
#include "cdc_desc.h"
#include "bat_brd.h"
#include "uart.h"
#include "at32f415.h"
#include "system_at32f415.h"

otg_core_type otg_core_struct;

uint8_t usb_buffer_rx[256];

uint16_t cdc_send_frame(uint8_t *buff, uint16_t len)
{
    return usbx_cdc_write(buff, len);
}

void usb_gpio_config(void)
{
    gpio_init_type gpio_init_struct;

    crm_periph_clock_enable(OTG_PIN_GPIO_CLOCK, TRUE);
    gpio_default_para_init(&gpio_init_struct);

    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;

#ifdef USB_SOF_OUTPUT_ENABLE
    crm_periph_clock_enable(OTG_PIN_SOF_GPIO_CLOCK, TRUE);
    gpio_init_struct.gpio_pins = OTG_PIN_SOF;
    gpio_init(OTG_PIN_SOF_GPIO, &gpio_init_struct);
#endif

#ifndef USB_VBUS_IGNORE
    gpio_init_struct.gpio_pins = OTG_PIN_VBUS;
    gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init(OTG_PIN_GPIO, &gpio_init_struct);
#endif
}

void usb_clock48m_select(usb_clk48_s clk_s)
{
    (void)clk_s;

    switch (system_core_clock) {
    case 48000000:
        crm_usb_clock_div_set(CRM_USB_DIV_1);
        break;
    case 72000000:
        crm_usb_clock_div_set(CRM_USB_DIV_1_5);
        break;
    case 96000000:
        crm_usb_clock_div_set(CRM_USB_DIV_2);
        break;
    case 120000000:
        crm_usb_clock_div_set(CRM_USB_DIV_2_5);
        break;
    case 144000000:
        crm_usb_clock_div_set(CRM_USB_DIV_3);
        break;
    default:
        break;
    }
}

void usb_delay_ms(uint32_t ms)
{
    delay_ms((uint16_t)ms);
}

void usb_delay_us(uint32_t us)
{
    delay_us(us);
}

void usb_device_init(void)
{
    UINT status;

    if (usbx_stack_init_pre_hw() != UX_SUCCESS) {
        return;
    }

    usb_gpio_config();
    crm_periph_clock_enable(OTG_CLOCK, TRUE);
    usb_clock48m_select(USB_CLK_HEXT);
    nvic_irq_enable(OTG_IRQ, 0, 0);

    usbd_init(&otg_core_struct,
              USB_FULL_SPEED_CORE_ID,
              USB_ID,
              &usbd_usbx_class_handler,
              &cdc_desc_handler);

    status = usbx_dcd_register(&otg_core_struct);
    (void)status;
}

static void uart_pc_init_deferred(void)
{
    static uint8_t done;
    static ULONG wait_ms;

    if (done || UART_PC_IsReady()) {
        return;
    }

    if (usbx_cdc_configured()) {
        UART_PC_Init();
        done = 1;
        return;
    }

    if (wait_ms >= 5000U) {
        UART_PC_Init();
        done = 1;
    } else {
        wait_ms++;
    }
}

void usb_task(ULONG thread_input)
{
    uint16_t data_len;

    (void)thread_input;

    for (;;) {
        uart_pc_init_deferred();

        data_len = usbx_cdc_read(usb_buffer_rx, sizeof(usb_buffer_rx));
        if (data_len == 0) {
            tx_thread_sleep(2);
        }
    }
}

void OTG_IRQ_HANDLER(void)
{
    usbd_irq_handler(&otg_core_struct);
}
