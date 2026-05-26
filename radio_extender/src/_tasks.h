/********************************************************************************* 
* tasks.h * FreeRTOS задачи для радиоудлинителя UART 
* ******************************************************************************/
#ifndef TASKS_H
#define TASKS_H

#include "FreeRTOS.h"
#include "task.h"
void task_uart_rx(void *pvParameters);
void task_rf_send(void *pvParameters);
void task_rf_receive(void *pvParameters);void idle_task(void *pvParameters);
#endif /* TASKS_H */