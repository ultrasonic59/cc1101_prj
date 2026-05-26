/**************************************************************************/
/* Minimal Artery class handler — forwards completions to USBX DCD        */
/**************************************************************************/
#include "usbd_core.h"
#include "ux_api.h"
#include "ux_dcd_at32.h"
#include "ux_device_stack.h"

static usb_sts_type usbx_init(void *udev);
static usb_sts_type usbx_clear(void *udev);
static usb_sts_type usbx_setup(void *udev, usb_setup_type *setup);
static usb_sts_type usbx_ept0_tx(void *udev);
static usb_sts_type usbx_ept0_rx(void *udev);
static usb_sts_type usbx_in(void *udev, uint8_t ept_num);
static usb_sts_type usbx_out(void *udev, uint8_t ept_num);
static usb_sts_type usbx_sof(void *udev);
static usb_sts_type usbx_event(void *udev, usbd_event_type event);

usbd_class_handler usbd_usbx_class_handler =
{
    usbx_init,
    usbx_clear,
    usbx_setup,
    usbx_ept0_tx,
    usbx_ept0_rx,
    usbx_in,
    usbx_out,
    usbx_sof,
    usbx_event,
    UX_NULL
};

static usb_sts_type usbx_init(void *udev)
{
    (void)udev;
    return USB_OK;
}

static usb_sts_type usbx_clear(void *udev)
{
    (void)udev;
    return USB_OK;
}

static usb_sts_type usbx_setup(void *udev, usb_setup_type *setup)
{
    (void)udev;
    (void)setup;
    return USB_OK;
}

static usb_sts_type usbx_ept0_tx(void *udev)
{
    ux_dcd_at32_ep0_tx_complete((usbd_core_type *)udev);
    return USB_OK;
}

static usb_sts_type usbx_ept0_rx(void *udev)
{
    ux_dcd_at32_ep0_rx_complete((usbd_core_type *)udev);
    return USB_OK;
}

static usb_sts_type usbx_in(void *udev, uint8_t ept_num)
{
    ux_dcd_at32_in_complete((usbd_core_type *)udev, ept_num);
    return USB_OK;
}

static usb_sts_type usbx_out(void *udev, uint8_t ept_num)
{
    ux_dcd_at32_out_complete((usbd_core_type *)udev, ept_num);
    return USB_OK;
}

static usb_sts_type usbx_sof(void *udev)
{
    (void)udev;
    return USB_OK;
}

static usb_sts_type usbx_event(void *udev, usbd_event_type event)
{
    (void)udev;

    if (event == USBD_RESET_EVENT)
        _ux_device_stack_disconnect();

    return USB_OK;
}
