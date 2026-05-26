# USB CDC on Azure RTOS USBX (AT32F415)

## Architecture

- **USBX** — device stack, class **CDC ACM** (`common/usbx`).
- **`ux_dcd_at32`** — DCD on Artery `usbd_core` / OTG FS (no STM32 HAL).
- **`usbd_usbx_class`** — IRQ completion hooks for USBX transfers.
- **`usb_thr.c`** — `usbx_cdc_read` / `usbx_cdc_write` application API.

Setup packets are handled by USBX (`usbd_core_setup_handler` → `ux_dcd_at32_setup_handler` when `UX_INCLUDE_USER_DEFINE_FILE` is defined).

## Build (IAR)

1. Run once: `python tools/patch_ewp_usbx.py` (adds `usbx` group, include paths, defines).
2. Rebuild `radio_extender`.

## RAM

`usbx_app.c` allocates **12 KiB** for USBX (`USBX_MEMORY_SIZE`). Reduce only if you accept enumeration/transfer failures.

## Hardware

Same as Artery VCP: PA11/PA12, `USB_VBUS_IGNORE` when PA9 is USART1 TX (MCU rev C).
