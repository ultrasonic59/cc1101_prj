#ifndef USBX_CDC_H
#define USBX_CDC_H

#include <stdint.h>

void usbx_cdc_init(void);
uint16_t usbx_cdc_write(const uint8_t *data, uint16_t len);
uint16_t usbx_cdc_read(uint8_t *buf, uint16_t max_len);
uint8_t usbx_cdc_configured(void);

#endif /* USBX_CDC_H */
