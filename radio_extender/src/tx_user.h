/**************************************************************************/
/* ThreadX user configuration for AT32F415 radio_extender (IAR, Cortex-M4) */
/**************************************************************************/

#ifndef TX_USER_H
#define TX_USER_H

/* Must be >= 32: kernel bit-maps use TX_MAX_PRIORITIES/32 array elements */
#define TX_MAX_PRIORITIES                       32
#define TX_TIMER_TICKS_PER_SECOND               1000
#define TX_MINIMUM_STACK                        256

/* System timer thread (priority 0 in port). Needs headroom when waking from SysTick. */
#define TX_TIMER_THREAD_STACK_SIZE              2048

/* Smaller kernel footprint on 32 KB RAM */
#define TX_DISABLE_PREEMPTION_THRESHOLD
#define TX_DISABLE_NOTIFY_CALLBACKS
#define TX_DISABLE_REDUNDANT_CLEARING

#endif /* TX_USER_H */
