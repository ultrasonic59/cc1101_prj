#ifndef USBX_APP_H
#define USBX_APP_H

#include "ux_api.h"
#include "ux_device_class_cdc_acm.h"
#include "usb_core.h"

UINT usbx_stack_init_pre_hw(void);
UINT usbx_dcd_register(otg_core_type *otg);
UX_SLAVE_CLASS_CDC_ACM *usbx_cdc_acm_instance(void);

#endif /* USBX_APP_H */
