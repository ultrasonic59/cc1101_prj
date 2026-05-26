#include "at32f415.h"
#include "tx_api.h"
#include "system_at32f415.h"

/* Called from main after system_clock_config(); reloads SysTick for actual SYSCLK. */
void tx_systick_config(void)
{
    uint32_t ticks = system_core_clock / TX_TIMER_TICKS_PER_SECOND;

    if (ticks == 0U) {
        ticks = 1U;
    }
    ticks--;

    SysTick->LOAD = ticks;
    SysTick->VAL  = 0U;
    SysTick->CTRL = (1U << 0) | (1U << 1) | (1U << 2);
}
