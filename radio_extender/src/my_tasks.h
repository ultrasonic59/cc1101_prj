/*********************************************************************************
 * my_tasks.h
 * ThreadX tasks for UART/USB <-> RF bridge
 ******************************************************************************/
#ifndef MY_TASKS_H
#define MY_TASKS_H

#include "tx_api.h"
#include <stdint.h>

void task_uart_rx(ULONG thread_input);
void task_rf_send(ULONG thread_input);
void task_rf_receive(ULONG thread_input);
void task_heartbeat(ULONG thread_input);
void tasks_init(void);

/* COBS-кадры с хоста (USB CDC bulk OUT / UART), внутри — кадр протокола DATA */
void bridge_host_rx_bytes(const uint8_t *data, size_t len);

#endif /* MY_TASKS_H */
