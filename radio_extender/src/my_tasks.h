/*********************************************************************************
 * my_tasks.h
 * ThreadX tasks for UART radio extender
 ******************************************************************************/
#ifndef MY_TASKS_H
#define MY_TASKS_H

#include "tx_api.h"

void task_uart_rx(ULONG thread_input);
void task_rf_send(ULONG thread_input);
void task_rf_receive(ULONG thread_input);
void task_heartbeat(ULONG thread_input);
void tasks_init(void);

#endif /* MY_TASKS_H */
