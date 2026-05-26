#!/usr/bin/env python3
"""Append USBX sources and include paths to radio_extender.ewp (idempotent)."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
EWP = ROOT / "radio_extender.ewp"
COMMON = ROOT.parent / "common" / "usbx"

CORE_SRC = COMMON / "common" / "core" / "src"
CDC_SRC = COMMON / "common" / "usbx_device_classes" / "src"
PORT_INC = COMMON / "ports" / "cortex_m4" / "iar" / "inc"

INCLUDE_PATHS = [
    r"$PROJ_DIR$\..\common\usbx\common\core\inc",
    r"$PROJ_DIR$\..\common\usbx\common\usbx_device_classes\inc",
    r"$PROJ_DIR$\..\common\usbx\ports\cortex_m4\iar\inc",
    r"$PROJ_DIR$\src\usbx",
]

DEFINES = ["UX_INCLUDE_USER_DEFINE_FILE", "UX_SOURCE_CODE"]

DEVICE_STACK = [
    "ux_device_stack_alternate_setting_get.c",
    "ux_device_stack_alternate_setting_set.c",
    "ux_device_stack_class_register.c",
    "ux_device_stack_class_unregister.c",
    "ux_device_stack_clear_feature.c",
    "ux_device_stack_configuration_get.c",
    "ux_device_stack_configuration_set.c",
    "ux_device_stack_control_request_process.c",
    "ux_device_stack_descriptor_send.c",
    "ux_device_stack_disconnect.c",
    "ux_device_stack_endpoint_stall.c",
    "ux_device_stack_get_status.c",
    "ux_device_stack_host_wakeup.c",
    "ux_device_stack_initialize.c",
    "ux_device_stack_interface_delete.c",
    "ux_device_stack_interface_get.c",
    "ux_device_stack_interface_set.c",
    "ux_device_stack_interface_start.c",
    "ux_device_stack_microsoft_extension_register.c",
    "ux_device_stack_set_feature.c",
    "ux_device_stack_tasks_run.c",
    "ux_device_stack_transfer_abort.c",
    "ux_device_stack_transfer_all_request_abort.c",
    "ux_device_stack_transfer_request.c",
    "ux_device_stack_transfer_run.c",
    "ux_device_stack_uninitialize.c",
]

UTILITY = [
    "ux_system_error_handler.c",
    "ux_system_initialize.c",
    "ux_system_tasks_run.c",
    "ux_system_uninitialize.c",
    "ux_utility_delay_ms.c",
    "ux_utility_descriptor_pack.c",
    "ux_utility_descriptor_parse.c",
    "ux_utility_event_flags_create.c",
    "ux_utility_event_flags_delete.c",
    "ux_utility_event_flags_get.c",
    "ux_utility_event_flags_set.c",
    "ux_utility_long_get.c",
    "ux_utility_long_get_big_endian.c",
    "ux_utility_long_put.c",
    "ux_utility_long_put_big_endian.c",
    "ux_utility_memory_allocate.c",
    "ux_utility_memory_compare.c",
    "ux_utility_memory_copy.c",
    "ux_utility_memory_free.c",
    "ux_utility_memory_free_block_best_get.c",
    "ux_utility_memory_set.c",
    "ux_utility_mutex_create.c",
    "ux_utility_mutex_delete.c",
    "ux_utility_mutex_off.c",
    "ux_utility_mutex_on.c",
    "ux_utility_semaphore_create.c",
    "ux_utility_semaphore_delete.c",
    "ux_utility_semaphore_get.c",
    "ux_utility_semaphore_put.c",
    "ux_utility_short_get.c",
    "ux_utility_short_put.c",
    "ux_utility_string_length_check.c",
]

CDC_ACM = [
    "ux_device_class_cdc_acm_activate.c",
    "ux_device_class_cdc_acm_bulkin_thread.c",
    "ux_device_class_cdc_acm_bulkout_thread.c",
    "ux_device_class_cdc_acm_control_request.c",
    "ux_device_class_cdc_acm_deactivate.c",
    "ux_device_class_cdc_acm_entry.c",
    "ux_device_class_cdc_acm_initialize.c",
    "ux_device_class_cdc_acm_ioctl.c",
    "ux_device_class_cdc_acm_read.c",
    "ux_device_class_cdc_acm_tasks_run.c",
    "ux_device_class_cdc_acm_unitialize.c",
    "ux_device_class_cdc_acm_write.c",
]

LOCAL = [
    "ux_dcd_at32.c",
    "usbd_usbx_class.c",
    "usbx_app.c",
    "usbx_cdc.c",
    "usbx_descriptors.c",
]


def file_xml(rel: str) -> str:
    rel = rel.replace("/", "\\")
    return f"        <file>\n            <name>$PROJ_DIR$\\{rel}</name>\n        </file>\n"


def main():
    text = EWP.read_text(encoding="utf-8")
    if "<name>usbx</name>" in text:
        print("ewp already patched")
        return

    for p in INCLUDE_PATHS:
        if p not in text:
            text = text.replace(
                "                    <state>$PROJ_DIR$\\src</state>\n                </option>",
                f"                    <state>$PROJ_DIR$\\src</state>\n                    <state>{p}</state>\n                </option>",
                1,
            )

    if "UX_INCLUDE_USER_DEFINE_FILE" not in text:
        text = text.replace(
            "                    <state>AT32F415RCT7</state>\n                </option>",
            "                    <state>AT32F415RCT7</state>\n"
            "                    <state>UX_INCLUDE_USER_DEFINE_FILE</state>\n"
            "                    <state>UX_SOURCE_CODE</state>\n"
            "                </option>",
            1,
        )

    group = ['    <group>\n        <name>usbx</name>\n']
    for f in DEVICE_STACK + UTILITY:
        group.append(file_xml(f"..\\common\\usbx\\common\\core\\src\\{f}"))
    for f in CDC_ACM:
        group.append(file_xml(f"..\\common\\usbx\\common\\usbx_device_classes\\src\\{f}"))
    for f in LOCAL:
        group.append(file_xml(f"src\\usbx\\{f}"))
    group.append("    </group>\n")

    text = text.replace("    </group>\n</project>", "".join(group) + "    </group>\n</project>", 1)
    EWP.write_text(text, encoding="utf-8")
    print("patched", EWP)


if __name__ == "__main__":
    main()
