#include "usbx_cdc.h"
#include "usbx_app.h"
#include "ux_device_class_cdc_acm.h"
#include "ux_api.h"

void usbx_cdc_init(void)
{
}

uint8_t usbx_cdc_configured(void)
{
    UX_SLAVE_DEVICE *device = &_ux_system_slave->ux_system_slave_device;

    return (device->ux_slave_device_state == UX_DEVICE_CONFIGURED) ? 1u : 0u;
}

uint16_t usbx_cdc_write(const uint8_t *data, uint16_t len)
{
    UX_SLAVE_CLASS_CDC_ACM *cdc = usbx_cdc_acm_instance();
    ULONG actual = 0;
    UINT status;

    if (!usbx_cdc_configured() || cdc == UX_NULL || data == UX_NULL || len == 0)
        return 0;

    status = ux_device_class_cdc_acm_write(cdc, (UCHAR *)data, len, &actual);
    if (status != UX_SUCCESS)
        return 0;

    return (uint16_t)actual;
}

uint16_t usbx_cdc_read(uint8_t *buf, uint16_t max_len)
{
    UX_SLAVE_CLASS_CDC_ACM *cdc = usbx_cdc_acm_instance();
    ULONG actual = 0;
    UINT status;

    if (!usbx_cdc_configured() || cdc == UX_NULL || buf == UX_NULL || max_len == 0)
        return 0;

    status = ux_device_class_cdc_acm_read(cdc, buf, max_len, &actual);
    if (status != UX_SUCCESS)
        return 0;

    return (uint16_t)actual;
}
