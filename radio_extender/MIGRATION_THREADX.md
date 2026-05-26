# Миграция radio_extender: FreeRTOS → ThreadX

## Что сделано

- RTOS заменён на **Azure RTOS ThreadX** (порт `cortex_m3` / IAR, подходит для Cortex-M4 AT32F415).
- Исходники ThreadX: junction `common/threadx` → `D:\proj\velograph\test_azure\threadx`.
- Задачи переписаны на `tx_thread_*`, `tx_queue_*`, `tx_thread_sleep`.
- Точка входа RTOS: `tx_kernel_enter()` + `tx_application_define()` в `src/tx_application.c`.
- Низкоуровневая инициализация: `src/tx_initialize_low_level.s` (SysTick 144 MHz, 1000 тик/с).
- Конфигурация: `src/tx_user.h` (`TX_TIMER_TICKS_PER_SECOND = 1000`).
- Линкер: `linker/AT32F415xC_threadx.icf` (секция `FREE_MEM` для кучи ThreadX).
- Проект IAR: группа `threadx` вместо `freertos` в `radio_extender.ewp`.

## Сборка

1. Открыть `radio_extender/radio_extender.eww` в IAR EWARM 9.x.
2. Убедиться, что существует junction `cc1101_prj/common/threadx`.
3. Rebuild All.

## Соответствие API

| FreeRTOS | ThreadX |
|----------|---------|
| `xTaskCreate` | `tx_thread_create` (+ стек из byte pool) |
| `vTaskStartScheduler` | `tx_kernel_enter` |
| `vTaskDelay(n)` | `tx_thread_sleep(n)` |
| `xQueueCreate/Send/Receive` | `tx_queue_create/send/receive` |
| `taskENTER_CRITICAL` | не используется (создание задач в `tx_application_define`) |
| `vApplicationIdleHook` | обновление LED перенесено в `task_heartbeat` |

## Приоритеты

В ThreadX **0 — наивысший** приоритет. Heartbeat = 0, RF send/receive = 1.

## Замечания

- В `tasks_init` создаются задачи `UART_RX` и `USB` (`tx_thread_create`), перед этим вызывается `UART_PC_Init()`.
- Для USB из ISR соблюдайте правила ThreadX (`tx_queue_send` из ISR только с `TX_NO_WAIT` и т.д.).
