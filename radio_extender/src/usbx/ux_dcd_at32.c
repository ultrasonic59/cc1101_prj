/**************************************************************************/
/* USBX DCD for AT32 — Artery usbd_core hardware layer                    */
/**************************************************************************/

#include "ux_api.h"
#include "ux_dcd_at32.h"
#include "ux_device_stack.h"
#include "ux_utility.h"
#include "usb_std.h"

static UX_DCD_AT32 *g_dcd_at32;

usbd_core_type *ux_dcd_at32_udev_get(void)
{
    if (g_dcd_at32 == UX_NULL)
        return UX_NULL;
    return g_dcd_at32->ux_dcd_at32_udev;
}

static UX_DCD_AT32_ED *at32_ed_get(UCHAR endpoint_address)
{
    ULONG index = endpoint_address & (ULONG)~UX_ENDPOINT_DIRECTION;

    if (index >= UX_DCD_AT32_MAX_ED)
        return UX_NULL;
    return &g_dcd_at32->ux_dcd_at32_ed[index];
}

static VOID at32_transfer_notify(UX_SLAVE_TRANSFER *transfer_request, ULONG actual_length)
{
    if (transfer_request == UX_NULL)
        return;

    transfer_request->ux_slave_transfer_request_actual_length = actual_length;
    transfer_request->ux_slave_transfer_request_completion_code = UX_SUCCESS;
    _ux_utility_semaphore_put(&transfer_request->ux_slave_transfer_request_semaphore);
}

static VOID at32_setup_in(UX_DCD_AT32_ED *ed, UX_SLAVE_TRANSFER *transfer_request)
{
    ed->ux_dcd_at32_ed_direction = UX_ENDPOINT_IN;
    ed->ux_dcd_at32_ed_state = UX_DCD_AT32_ED_STATE_DATA_TX;
    _ux_device_stack_control_request_process(transfer_request);
}

static VOID at32_setup_out(UX_DCD_AT32_ED *ed, UX_SLAVE_TRANSFER *transfer_request,
                           usbd_core_type *udev)
{
    transfer_request->ux_slave_transfer_request_completion_code = UX_SUCCESS;
    ed->ux_dcd_at32_ed_direction = UX_ENDPOINT_IN;

    if (_ux_device_stack_control_request_process(transfer_request) == UX_SUCCESS)
    {
        ed->ux_dcd_at32_ed_state = UX_DCD_AT32_ED_STATE_STATUS_TX;
        usbd_ctrl_send_status(udev);
    }
}

static VOID at32_setup_status(UX_DCD_AT32_ED *ed, UX_SLAVE_TRANSFER *transfer_request,
                              usbd_core_type *udev)
{
    ed->ux_dcd_at32_ed_direction = UX_ENDPOINT_IN;

    if (_ux_device_stack_control_request_process(transfer_request) == UX_SUCCESS)
    {
        ed->ux_dcd_at32_ed_state = UX_DCD_AT32_ED_STATE_STATUS_RX;
        usbd_ctrl_send_status(udev);
    }
}

void ux_dcd_at32_setup_handler(usbd_core_type *udev)
{
    UX_DCD_AT32_ED       *ed;
    UX_SLAVE_TRANSFER    *transfer_request;
    usb_setup_type       *setup = &udev->setup;

    if (g_dcd_at32 == UX_NULL)
        return;

    ed = &g_dcd_at32->ux_dcd_at32_ed[0];
    transfer_request = &ed->ux_dcd_at32_ed_endpoint->ux_slave_endpoint_transfer_request;

    transfer_request->ux_slave_transfer_request_setup[0] = setup->bmRequestType;
    transfer_request->ux_slave_transfer_request_setup[1] = setup->bRequest;
    transfer_request->ux_slave_transfer_request_setup[2] = (UCHAR)(setup->wValue & 0xFF);
    transfer_request->ux_slave_transfer_request_setup[3] = (UCHAR)(setup->wValue >> 8);
    transfer_request->ux_slave_transfer_request_setup[4] = (UCHAR)(setup->wIndex & 0xFF);
    transfer_request->ux_slave_transfer_request_setup[5] = (UCHAR)(setup->wIndex >> 8);
    transfer_request->ux_slave_transfer_request_setup[6] = (UCHAR)(setup->wLength & 0xFF);
    transfer_request->ux_slave_transfer_request_setup[7] = (UCHAR)(setup->wLength >> 8);

    transfer_request->ux_slave_transfer_request_actual_length = 0;
    transfer_request->ux_slave_transfer_request_type = UX_TRANSFER_PHASE_SETUP;
    transfer_request->ux_slave_transfer_request_completion_code = UX_SUCCESS;

    if (setup->bmRequestType & UX_REQUEST_IN)
    {
        at32_setup_in(ed, transfer_request);
    }
    else
    {
        ed->ux_dcd_at32_ed_direction = UX_ENDPOINT_OUT;

        if (setup->wLength == 0)
            at32_setup_status(ed, transfer_request, udev);
        else
        {
            transfer_request->ux_slave_transfer_request_requested_length = setup->wLength;
            transfer_request->ux_slave_transfer_request_current_data_pointer =
                transfer_request->ux_slave_transfer_request_data_pointer;
            ed->ux_dcd_at32_ed_state = UX_DCD_AT32_ED_STATE_DATA_RX;
            usbd_ctrl_recv(udev,
                             transfer_request->ux_slave_transfer_request_data_pointer,
                             (uint16_t)setup->wLength);
        }
    }
}

void ux_dcd_at32_ep0_tx_complete(usbd_core_type *udev)
{
    UX_DCD_AT32_ED    *ed;
    UX_SLAVE_TRANSFER *transfer_request;

    (void)udev;
    if (g_dcd_at32 == UX_NULL)
        return;

    ed = &g_dcd_at32->ux_dcd_at32_ed[0];
    transfer_request = &ed->ux_dcd_at32_ed_endpoint->ux_slave_endpoint_transfer_request;

    switch (ed->ux_dcd_at32_ed_state)
    {
    case UX_DCD_AT32_ED_STATE_DATA_TX:
        at32_transfer_notify(transfer_request, transfer_request->ux_slave_transfer_request_requested_length);
        ed->ux_dcd_at32_ed_state = UX_DCD_AT32_ED_STATE_IDLE;
        break;

    case UX_DCD_AT32_ED_STATE_STATUS_TX:
        at32_transfer_notify(transfer_request, 0);
        ed->ux_dcd_at32_ed_state = UX_DCD_AT32_ED_STATE_IDLE;
        break;

    default:
        break;
    }
}

void ux_dcd_at32_ep0_rx_complete(usbd_core_type *udev)
{
    UX_DCD_AT32_ED    *ed;
    UX_SLAVE_TRANSFER *transfer_request;
    ULONG              len;
    UX_SLAVE_ENDPOINT *endpoint;

    if (g_dcd_at32 == UX_NULL)
        return;

    ed = &g_dcd_at32->ux_dcd_at32_ed[0];
    transfer_request = &ed->ux_dcd_at32_ed_endpoint->ux_slave_endpoint_transfer_request;
    len = usbd_get_recv_len(udev, 0);

    switch (ed->ux_dcd_at32_ed_state)
    {
    case UX_DCD_AT32_ED_STATE_DATA_RX:
        endpoint = transfer_request->ux_slave_transfer_request_endpoint;
        transfer_request->ux_slave_transfer_request_actual_length += len;

        if (transfer_request->ux_slave_transfer_request_actual_length <
                transfer_request->ux_slave_transfer_request_requested_length &&
            len == endpoint->ux_slave_endpoint_descriptor.wMaxPacketSize)
        {
            transfer_request->ux_slave_transfer_request_current_data_pointer += len;
            usbd_ctrl_recv(udev,
                             transfer_request->ux_slave_transfer_request_current_data_pointer,
                             (uint16_t)(transfer_request->ux_slave_transfer_request_requested_length -
                                        transfer_request->ux_slave_transfer_request_actual_length));
        }
        else
            at32_setup_out(ed, transfer_request, udev);
        break;

    default:
        break;
    }
}

void ux_dcd_at32_in_complete(usbd_core_type *udev, uint8_t ept_num)
{
    UX_DCD_AT32_ED    *ed;
    UX_SLAVE_TRANSFER *transfer_request;
    usb_ept_info      *ept_info;

    (void)udev;
    if (g_dcd_at32 == UX_NULL)
        return;

    ed = at32_ed_get(ept_num | 0x80u);
    if (ed == UX_NULL || (ed->ux_dcd_at32_ed_status & UX_DCD_AT32_ED_STATUS_TRANSFER) == 0)
        return;

    ept_info = &g_dcd_at32->ux_dcd_at32_udev->ept_in[ept_num & 0x7Fu];
    transfer_request = &ed->ux_dcd_at32_ed_endpoint->ux_slave_endpoint_transfer_request;
    ed->ux_dcd_at32_ed_status &= (ULONG)~UX_DCD_AT32_ED_STATUS_TRANSFER;
    at32_transfer_notify(transfer_request, ept_info->trans_len);
}

void ux_dcd_at32_out_complete(usbd_core_type *udev, uint8_t ept_num)
{
    UX_DCD_AT32_ED    *ed;
    UX_SLAVE_TRANSFER *transfer_request;
    ULONG              len;

    if (g_dcd_at32 == UX_NULL)
        return;

    ed = at32_ed_get(ept_num);
    if (ed == UX_NULL || (ed->ux_dcd_at32_ed_status & UX_DCD_AT32_ED_STATUS_TRANSFER) == 0)
        return;

    len = usbd_get_recv_len(udev, ept_num);
    transfer_request = &ed->ux_dcd_at32_ed_endpoint->ux_slave_endpoint_transfer_request;
    ed->ux_dcd_at32_ed_status &= (ULONG)~UX_DCD_AT32_ED_STATUS_TRANSFER;
    at32_transfer_notify(transfer_request, len);
}

UINT _ux_dcd_at32_initialize(ULONG dcd_io, ULONG parameter)
{
    UX_SLAVE_DCD *dcd;
    UX_DCD_AT32  *dcd_at32;

    UX_PARAMETER_NOT_USED(dcd_io);

    dcd = &_ux_system_slave->ux_system_slave_dcd;
    dcd->ux_slave_dcd_controller_type = UX_DCD_AT32_SLAVE_CONTROLLER;

    dcd_at32 = _ux_utility_memory_allocate(UX_NO_ALIGN, UX_REGULAR_MEMORY, sizeof(UX_DCD_AT32));
    if (dcd_at32 == UX_NULL)
        return UX_MEMORY_INSUFFICIENT;

    _ux_utility_memory_set(dcd_at32, 0, sizeof(UX_DCD_AT32));
    dcd->ux_slave_dcd_controller_hardware = dcd_at32;
    dcd_at32->ux_dcd_at32_dcd_owner = dcd;
    dcd_at32->ux_dcd_at32_udev = (usbd_core_type *)parameter;
    dcd->ux_slave_dcd_function = _ux_dcd_at32_function;
    g_dcd_at32 = dcd_at32;
    dcd->ux_slave_dcd_status = UX_DCD_STATUS_OPERATIONAL;

    return UX_SUCCESS;
}

static UINT at32_endpoint_create(UX_DCD_AT32 *dcd_at32, UX_SLAVE_ENDPOINT *endpoint)
{
    UX_DCD_AT32_ED *ed;
    ULONG           index;
    UCHAR           addr;
    UCHAR           type;
    USHORT          mps;

    addr = endpoint->ux_slave_endpoint_descriptor.bEndpointAddress;
    index = addr & (ULONG)~UX_ENDPOINT_DIRECTION;
    if (index >= UX_DCD_AT32_MAX_ED)
        return UX_NO_ED_AVAILABLE;

    ed = &dcd_at32->ux_dcd_at32_ed[index];
    if (ed->ux_dcd_at32_ed_status & UX_DCD_AT32_ED_STATUS_USED)
        return UX_ERROR;

    ed->ux_dcd_at32_ed_status |= UX_DCD_AT32_ED_STATUS_USED;
    ed->ux_dcd_at32_ed_index = (UCHAR)index;
    ed->ux_dcd_at32_ed_endpoint = endpoint;
    endpoint->ux_slave_endpoint_ed = ed;

    mps = endpoint->ux_slave_endpoint_descriptor.wMaxPacketSize;
    switch (endpoint->ux_slave_endpoint_descriptor.bmAttributes & UX_MASK_ENDPOINT_TYPE)
    {
    case UX_CONTROL_ENDPOINT:
        type = EPT_CONTROL_TYPE;
        break;
    case UX_BULK_ENDPOINT:
        type = EPT_BULK_TYPE;
        break;
    case UX_INTERRUPT_ENDPOINT:
        type = EPT_INT_TYPE;
        break;
    default:
        type = EPT_ISO_TYPE;
        break;
    }

    usbd_ept_open(dcd_at32->ux_dcd_at32_udev, addr, type, mps);
    return UX_SUCCESS;
}

static UINT at32_endpoint_destroy(UX_DCD_AT32 *dcd_at32, UX_SLAVE_ENDPOINT *endpoint)
{
    UX_DCD_AT32_ED *ed = (UX_DCD_AT32_ED *)endpoint->ux_slave_endpoint_ed;

    if (ed == UX_NULL)
        return UX_ERROR;

    usbd_ept_close(dcd_at32->ux_dcd_at32_udev,
                   endpoint->ux_slave_endpoint_descriptor.bEndpointAddress);
    ed->ux_dcd_at32_ed_status = UX_DCD_AT32_ED_STATUS_UNUSED;
    endpoint->ux_slave_endpoint_ed = UX_NULL;
    return UX_SUCCESS;
}

static UINT at32_transfer_request(UX_DCD_AT32 *dcd_at32, UX_SLAVE_TRANSFER *transfer_request)
{
    UX_SLAVE_ENDPOINT *endpoint;
    UX_DCD_AT32_ED    *ed;
    UCHAR              addr;
    UINT               status;

    endpoint = transfer_request->ux_slave_transfer_request_endpoint;
    addr = endpoint->ux_slave_endpoint_descriptor.bEndpointAddress;
    ed = (UX_DCD_AT32_ED *)endpoint->ux_slave_endpoint_ed;

    if (ed == UX_NULL)
        return UX_ERROR;

    ed->ux_dcd_at32_ed_status |= UX_DCD_AT32_ED_STATUS_TRANSFER;

    if (transfer_request->ux_slave_transfer_request_phase == UX_TRANSFER_PHASE_DATA_OUT)
    {
        if ((addr & 0x7Fu) == 0)
            ed->ux_dcd_at32_ed_state = UX_DCD_AT32_ED_STATE_DATA_TX;

        usbd_ept_send(dcd_at32->ux_dcd_at32_udev, addr,
                      transfer_request->ux_slave_transfer_request_data_pointer,
                      (uint16_t)transfer_request->ux_slave_transfer_request_requested_length);
    }
    else
    {
        if ((addr & 0x7Fu) == 0)
            ed->ux_dcd_at32_ed_state = UX_DCD_AT32_ED_STATE_DATA_RX;

        usbd_ept_recv(dcd_at32->ux_dcd_at32_udev, addr,
                      transfer_request->ux_slave_transfer_request_data_pointer,
                      (uint16_t)transfer_request->ux_slave_transfer_request_requested_length);
    }

    if ((addr & 0x7Fu) != 0)
        return UX_SUCCESS;

    status = _ux_utility_semaphore_get(&transfer_request->ux_slave_transfer_request_semaphore,
                                       transfer_request->ux_slave_transfer_request_timeout);
    if (status != UX_SUCCESS)
        return status;
    return transfer_request->ux_slave_transfer_request_completion_code;
}

UINT _ux_dcd_at32_function(UX_SLAVE_DCD *dcd, UINT function, VOID *parameter)
{
    UX_DCD_AT32 *dcd_at32 = (UX_DCD_AT32 *)dcd->ux_slave_dcd_controller_hardware;

    if (dcd->ux_slave_dcd_status == UX_UNUSED)
        return UX_CONTROLLER_UNKNOWN;

    switch (function)
    {
    case UX_DCD_GET_FRAME_NUMBER:
    case UX_DCD_TRANSFER_ABORT:
    case UX_DCD_RESET_ENDPOINT:
    case UX_DCD_ENDPOINT_STATUS:
    case UX_DCD_SET_DEVICE_ADDRESS:
        return UX_FUNCTION_NOT_SUPPORTED;

    case UX_DCD_CREATE_ENDPOINT:
        return at32_endpoint_create(dcd_at32, (UX_SLAVE_ENDPOINT *)parameter);

    case UX_DCD_DESTROY_ENDPOINT:
        return at32_endpoint_destroy(dcd_at32, (UX_SLAVE_ENDPOINT *)parameter);

    case UX_DCD_STALL_ENDPOINT:
    {
        UCHAR addr = ((UX_SLAVE_ENDPOINT *)parameter)->ux_slave_endpoint_descriptor.bEndpointAddress;
        usbd_set_stall(dcd_at32->ux_dcd_at32_udev, addr);
        return UX_SUCCESS;
    }

    case UX_DCD_TRANSFER_REQUEST:
        return at32_transfer_request(dcd_at32, (UX_SLAVE_TRANSFER *)parameter);

    default:
        return UX_FUNCTION_NOT_SUPPORTED;
    }
}
