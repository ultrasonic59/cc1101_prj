/*********************************************************************************
 * main.c - UART radio extender entry (AT32F415RCT7 + CC1101), ThreadX
 ******************************************************************************/
#include <stdio.h>
#include "at32f415.h"
#include "system_at32f415.h"
#include "tx_api.h"
#include "uart.h"
#include "cc1101.h"
#include "my_tasks.h"
#include "bat_brd.h"
#include "printk.h"
#include "usb_thr.h"

extern int send_char_dbg(int c);
void tx_systick_config(void);

int main(void)
{
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    hw_board_init();
    printk("\n\r CC1101 prj");
/*
    if (!CC1101_ProbeSpi()) {
        put_led(LED_RED);
        delay_ms(800);
        put_led(LED_OFF);
    }
*/
    CC1101_Init();

    /* USB до USART1 на PA9 — как рабочий FreeRTOS (velograph) */
    usb_device_init();

    CC1101_ReapplyRadio();

  ///  if (CC1101_CheckConnection()) {
    if (CC1101_VerifySpi()) {
        put_led(LED_GREEN);
        delay_ms(300);
        put_led(LED_RED);
    } else {
        put_led(LED_RED);
    ///    delay_ms(300);
        printk("\n\r CC1101 verify FAIL (CHANNR readback)");
            for (;;) {
    }

    }

    tx_systick_config();
    tx_kernel_enter();

    for (;;) {
    }
    return 0;
}
