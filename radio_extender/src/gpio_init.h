/********************************************************************************* 
* gpio_init.h 
* Инициализация GPIO для радиоудлинителя UART * AT32F415RCT7 + CC1101 
* ******************************************************************************/
#ifndef __GPIO_INIT_H__
#define __GPIO_INIT_H__

extern void GPIO_Init(void);

extern void LED_UART_RX_On(void); 
extern void LED_UART_RX_Off(void);
 
extern void LED_RF_TX_On(void) ;
extern void LED_RF_TX_Off(void) ; 
 
#endif