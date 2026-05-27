/**************************************************************************/
/* USBX device stack init — CDC ACM on AT32 OTG FS                        */
/**************************************************************************/

#include "usbx_app.h"
#include "ux_api.h"
#include "ux_device_stack.h"
#include "ux_device_class_cdc_acm.h"
#include "ux_dcd_at32.h"
#include "usb_core.h"
#include "cdc_desc.h"

#define USBX_MEMORY_SIZE        (12 * 1024)

static ULONG usbx_memory[USBX_MEMORY_SIZE / sizeof(ULONG)];

static UX_SLAVE_CLASS_CDC_ACM *g_cdc_acm;

static UCHAR string_framework[] = {
    0x09, 0x04, 0x01, 0x06, 'A', 'r', 't', 'e', 'r', 'y',
    0x09, 0x04, 0x02, 0x16,
    'A', 'T', '3', '2', ' ', 'V', 'i', 'r', 't', 'u', 'a', 'l', ' ',
    'C', 'O', 'M', ' ', ' ', ' ',
    0x09, 0x04, 0x03, 0x04, '0', '0', '0', '1'
};

extern const UCHAR *usbx_device_framework_get(ULONG *length);
extern const UCHAR *usbx_language_id_framework_get(ULONG *length);
extern void usbx_descriptors_init(void);

static VOID cdc_acm_activate(VOID *instance)
{
    g_cdc_acm = (UX_SLAVE_CLASS_CDC_ACM *)instance;
}

UINT usbx_stack_init_pre_hw(void)
{
    UINT status;
    ULONG df_len;
    ULONG lang_len;
    const UCHAR *df;
    const UCHAR *lang;
    UX_SLAVE_CLASS_CDC_ACM_PARAMETER param;

    usbx_descriptors_init();

    status = ux_system_initialize(usbx_memory, USBX_MEMORY_SIZE, UX_NULL, 0);
    if (status != UX_SUCCESS)
        return status;

    df = usbx_device_framework_get(&df_len);
    lang = usbx_language_id_framework_get(&lang_len);

    status = ux_device_stack_initialize((UCHAR *)df, df_len,
                                        (UCHAR *)df, df_len,
                                        string_framework, sizeof(string_framework),
                                        (UCHAR *)lang, lang_len,
                                        UX_NULL);
    if (status != UX_SUCCESS)
        return status;

    _ux_utility_memory_set(&param, 0, sizeof(param));
    param.ux_slave_class_cdc_acm_instance_activate = cdc_acm_activate;

    status = ux_device_stack_class_register(_ux_system_slave_class_cdc_acm_name,
                                            _ux_device_class_cdc_acm_entry,
                                            1, 0, &param);
    return status;
}

UINT usbx_dcd_register(otg_core_type *otg)
{
    return _ux_dcd_at32_initialize(0, (ULONG)&otg->dev);
}

UX_SLAVE_CLASS_CDC_ACM *usbx_cdc_acm_instance(void)
{
    return g_cdc_acm;
}
