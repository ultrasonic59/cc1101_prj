;/**************************************************************************/
;/*  ThreadX low-level init for AT32F415 @ 144 MHz, 1000 ticks/s           */
;/**************************************************************************/

    EXTERN  _tx_thread_system_stack_ptr
    EXTERN  _tx_initialize_unused_memory
    EXTERN  _tx_timer_interrupt
    EXTERN  __vector_table
    EXTERN  _tx_execution_isr_enter
    EXTERN  _tx_execution_isr_exit

SYSTEM_CLOCK      EQU   144000000
SYSTICK_CYCLES    EQU   ((SYSTEM_CLOCK / 1000) - 1)

    RSEG    FREE_MEM:DATA
    PUBLIC  __tx_free_memory_start
__tx_free_memory_start
    DS32    4

    SECTION `.text`:CODE:NOROOT(2)
    THUMB

    PUBLIC  _tx_initialize_low_level
_tx_initialize_low_level:

    CPSID   i

    LDR     r0, =__tx_free_memory_start
    LDR     r2, =_tx_initialize_unused_memory
    STR     r0, [r2, #0]

    LDR     r0, =0xE0001000
    LDR     r1, [r0]
    ORR     r1, r1, #1
    STR     r1, [r0]

    MOV     r0, #0xE000E000
    LDR     r1, =__vector_table
    STR     r1, [r0, #0xD08]

    LDR     r0, =_tx_thread_system_stack_ptr
    LDR     r1, =__vector_table
    LDR     r1, [r1]
    STR     r1, [r0]

    /* SysTick LOAD/CTRL: configured in tx_systick_config() from system_core_clock. */

    MOV     r0, #0xE000E000
    LDR     r1, =0x00000000
    STR     r1, [r0, #0xD18]

    LDR     r1, =0xFF000000
    STR     r1, [r0, #0xD1C]

    LDR     r1, =0x40FF0000
    STR     r1, [r0, #0xD20]

    BX      lr

    PUBLIC  SysTick_Handler
    PUBLIC  __tx_SysTickHandler
SysTick_Handler:
__tx_SysTickHandler:
    PUSH    {r0, lr}
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_enter
#endif
    BL      _tx_timer_interrupt
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_exit
#endif
    POP     {r0, lr}
    BX      LR

    END
