#ifndef __USB_THR_H__
#define __USB_THR_H__

#include "usb_core.h"
#include "tx_api.h"

extern otg_core_type otg_core_struct;

extern void usb_device_init(void);
extern void usb_task(ULONG thread_input);
extern uint16_t cdc_send_frame(uint8_t *buff, uint16_t len);

#endif
