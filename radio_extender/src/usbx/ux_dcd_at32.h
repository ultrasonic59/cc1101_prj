/**************************************************************************/
/* USBX device controller driver — AT32 OTG FS via Artery usbd_core        */
/**************************************************************************/
#ifndef UX_DCD_AT32_H
#define UX_DCD_AT32_H

#include "ux_api.h"
#include "usbd_core.h"

#define UX_DCD_AT32_SLAVE_CONTROLLER            0x41u
#ifndef UX_DCD_AT32_MAX_ED
#define UX_DCD_AT32_MAX_ED                      4
#endif

#define UX_DCD_AT32_ED_STATUS_UNUSED            0u
#define UX_DCD_AT32_ED_STATUS_USED              1u
#define UX_DCD_AT32_ED_STATUS_TRANSFER          2u

#define UX_DCD_AT32_ED_STATE_IDLE               0
#define UX_DCD_AT32_ED_STATE_DATA_TX            1
#define UX_DCD_AT32_ED_STATE_DATA_RX            2
#define UX_DCD_AT32_ED_STATE_STATUS_TX          3
#define UX_DCD_AT32_ED_STATE_STATUS_RX          4

typedef struct UX_DCD_AT32_ED_STRUCT
{
    struct UX_SLAVE_ENDPOINT_STRUCT *ux_dcd_at32_ed_endpoint;
    ULONG                           ux_dcd_at32_ed_status;
    UCHAR                           ux_dcd_at32_ed_state;
    UCHAR                           ux_dcd_at32_ed_index;
    UCHAR                           ux_dcd_at32_ed_direction;
} UX_DCD_AT32_ED;

typedef struct UX_DCD_AT32_STRUCT
{
    struct UX_SLAVE_DCD_STRUCT      *ux_dcd_at32_dcd_owner;
    usbd_core_type                  *ux_dcd_at32_udev;
    UX_DCD_AT32_ED                  ux_dcd_at32_ed[UX_DCD_AT32_MAX_ED];
} UX_DCD_AT32;

UINT  _ux_dcd_at32_initialize(ULONG dcd_io, ULONG parameter);
UINT  _ux_dcd_at32_function(UX_SLAVE_DCD *dcd, UINT function, VOID *parameter);

void  ux_dcd_at32_setup_handler(usbd_core_type *udev);
void  ux_dcd_at32_in_complete(usbd_core_type *udev, uint8_t ept_num);
void  ux_dcd_at32_out_complete(usbd_core_type *udev, uint8_t ept_num);
void  ux_dcd_at32_ep0_tx_complete(usbd_core_type *udev);
void  ux_dcd_at32_ep0_rx_complete(usbd_core_type *udev);

usbd_core_type *ux_dcd_at32_udev_get(void);

#endif /* UX_DCD_AT32_H */
