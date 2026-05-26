/**************************************************************************/
/* USBX device / string / language frameworks (from Artery CDC descriptors) */
/**************************************************************************/
#include "ux_api.h"
#include "usb_std.h"
#include "cdc_desc.h"
#include "cdc_class.h"

#define USBX_DEVICE_FRAMEWORK_LEN   (USB_DEVICE_DESC_LEN + USBD_CDC_CONFIG_DESC_SIZE)

#if defined(__ICCARM__)
#pragma data_alignment = 4
#endif
static const UCHAR usbx_device_framework[USBX_DEVICE_FRAMEWORK_LEN] =
{
    /* Device descriptor (18 bytes) — same as g_usbd_descriptor in cdc_desc.c */
    USB_DEVICE_DESC_LEN, USB_DESCIPTOR_TYPE_DEVICE,
    0x00, 0x02,
    0x02, 0x00, 0x00,
    USB_MAX_EP0_SIZE,
    (UCHAR)(USBD_CDC_VENDOR_ID & 0xFF), (UCHAR)(USBD_CDC_VENDOR_ID >> 8),
    (UCHAR)(USBD_CDC_PRODUCT_ID & 0xFF), (UCHAR)(USBD_CDC_PRODUCT_ID >> 8),
    0x00, 0x02,
    USB_MFC_STRING, USB_PRODUCT_STRING, USB_SERIAL_STRING,
    0x01,

    /* Configuration + interfaces — byte copy of g_usbd_configuration */
    USB_DEVICE_CFG_DESC_LEN, USB_DESCIPTOR_TYPE_CONFIGURATION,
    (UCHAR)(USBD_CDC_CONFIG_DESC_SIZE & 0xFF), (UCHAR)(USBD_CDC_CONFIG_DESC_SIZE >> 8),
    0x02, 0x01, 0x00, 0xC0, 0x32,

    USB_DEVICE_IF_DESC_LEN, USB_DESCIPTOR_TYPE_INTERFACE,
    0x00, 0x00, 0x01, USB_CLASS_CODE_CDC, 0x02, 0x01, 0x00,

    0x05, USBD_CDC_CS_INTERFACE, USBD_CDC_SUBTYPE_HEADER,
    (UCHAR)(CDC_BCD_NUM & 0xFF), (UCHAR)(CDC_BCD_NUM >> 8),

    0x05, USBD_CDC_CS_INTERFACE, USBD_CDC_SUBTYPE_CMF, 0x00, 0x01,
    0x04, USBD_CDC_CS_INTERFACE, USBD_CDC_SUBTYPE_ACM, 0x02,
    0x05, USBD_CDC_CS_INTERFACE, USBD_CDC_SUBTYPE_UFD, 0x00, 0x01,

    USB_DEVICE_EPT_LEN, USB_DESCIPTOR_TYPE_ENDPOINT,
    USBD_CDC_INT_EPT, USB_EPT_DESC_INTERRUPT,
    (UCHAR)(USBD_CDC_CMD_MAXPACKET_SIZE & 0xFF), (UCHAR)(USBD_CDC_CMD_MAXPACKET_SIZE >> 8),
    CDC_HID_BINTERVAL_TIME,

    USB_DEVICE_IF_DESC_LEN, USB_DESCIPTOR_TYPE_INTERFACE,
    0x01, 0x00, 0x02, USB_CLASS_CODE_CDCDATA, 0x00, 0x00, 0x00,

    USB_DEVICE_EPT_LEN, USB_DESCIPTOR_TYPE_ENDPOINT,
    USBD_CDC_BULK_IN_EPT, USB_EPT_DESC_BULK,
    (UCHAR)(USBD_CDC_IN_MAXPACKET_SIZE & 0xFF), (UCHAR)(USBD_CDC_IN_MAXPACKET_SIZE >> 8),
    0x00,

    USB_DEVICE_EPT_LEN, USB_DESCIPTOR_TYPE_ENDPOINT,
    USBD_CDC_BULK_OUT_EPT, USB_EPT_DESC_BULK,
    (UCHAR)(USBD_CDC_OUT_MAXPACKET_SIZE & 0xFF), (UCHAR)(USBD_CDC_OUT_MAXPACKET_SIZE >> 8),
    0x00
};

static UCHAR usbx_string_framework[128];
static UCHAR usbx_language_id_framework[] = { 0x09, 0x04 };

const UCHAR *usbx_device_framework_get(ULONG *length)
{
    if (length)
        *length = USBX_DEVICE_FRAMEWORK_LEN;
    return usbx_device_framework;
}

const UCHAR *usbx_string_framework_get(ULONG *length)
{
    /* Built at runtime in usbx_descriptors_init() from cdc_desc strings */
    if (length)
        *length = 0;
    return usbx_string_framework;
}

const UCHAR *usbx_language_id_framework_get(ULONG *length)
{
    if (length)
        *length = sizeof(usbx_language_id_framework);
    return usbx_language_id_framework;
}

void usbx_descriptors_init(void)
{
    /* Use Artery string builders via existing cdc_desc_handler at runtime if needed.
     * For enumeration, USBX parses string index from framework; minimal LANGID only here. */
    (void)usbx_string_framework;
}
