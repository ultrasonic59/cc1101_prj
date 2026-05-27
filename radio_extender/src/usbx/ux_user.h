/**************************************************************************/
/* USBX user config — AT32F415 device CDC ACM only (ThreadX)              */
/**************************************************************************/
#ifndef UX_USER_H
#define UX_USER_H

/* UX_INCLUDE_USER_DEFINE_FILE — задан в настройках IAR (CCDefines) */

/* Device-only build: no USB host stack / _ux_system_host */
#define UX_DEVICE_SIDE_ONLY

#define UX_MAX_SLAVE_INTERFACES                 2
#define UX_MAX_SLAVE_CLASS_DRIVER               1
#define UX_MAX_DEVICE_ENDPOINTS                 4

#define UX_SLAVE_REQUEST_CONTROL_MAX_LENGTH     512
#define UX_SLAVE_REQUEST_DATA_MAX_LENGTH        2048

#define UX_DEVICE_CLASS_CDC_ACM_READ_BUFFER_SIZE  256
#define UX_DEVICE_CLASS_CDC_ACM_WRITE_BUFFER_SIZE 256

#define UX_THREAD_STACK_SIZE                    1024

/* Smaller pool: 32 KB SRAM total on AT32F415RCT7 */
#define UX_DEVICE_CDC_ACM_NX_POOL_SIZE          0

#endif /* UX_USER_H */
