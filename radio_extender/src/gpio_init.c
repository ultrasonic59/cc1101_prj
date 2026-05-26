/********************************************************************************* 
* gpio_init.c 
* Инициализация GPIO для радиоудлинителя UART * AT32F415RCT7 + CC1101 
* ******************************************************************************/
#include "at32f415.h"

void GPIO_Init(void){
#if 0
  /* Включить тактирование GPIOA, GPIOB, GPIOC */    
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN; 
  /* === LED1 (PC13) и LED2 (PC14) === */    /* PC13, PC14 - push-pull, 2 MHz */   
  GPIOC->CRH &= ~(0xF << 20);  
  /* PC13 */  
  GPIOC->CRH |=  (0x2 << 20);   
  /* Output, push-pull, 2 MHz */  
  GPIOC->CRH &= ~(0xF << 24);   
  /* PC14 */    GPIOC->CRH |=  (0x2 << 24);  
  GPIOC->ODR |= (1 << 13) | (1 << 14); /* LED off (active low) */ 
  /* === UART2: PA2 (TX), PA3 (RX) === */  
  /* PA2 - Alternate function push-pull, 50 MHz */   
  GPIOA->CRL &= ~(0xF << 8);  
  GPIOA->CRL |=  (0xB << 8);  
  /* PA3 - Input floating */  
  GPIOA->CRL &= ~(0xF << 12); 
  GPIOA->CRL |=  (0x4 << 12);  
  /* === SPI2: PB12(NSS), PB13(SCK), PB14(MISO), PB15(MOSI) === */  
  /* PB12 - Output push-pull, 50 MHz (NSS) */ 
  GPIOB->CRH &= ~(0xF << 16);   
  GPIOB->CRH |=  (0x3 << 16);  
  /* PB13 - Alternate function push-pull, 50 MHz (SCK) */
  GPIOB->CRH &= ~(0xF << 20); 
  GPIOB->CRH |=  (0xB << 20); 
  /* PB14 - Input floating (MISO) */  
  GPIOB->CRH &= ~(0xF << 24);  
  GPIOB->CRH |=  (0x4 << 24); 
  /* PB15 - Alternate function push-pull, 50 MHz (MOSI) */  
  GPIOB->CRH &= ~(0xF << 28);  
  GPIOB->CRH |=  (0xB << 28);  
  /* === GDO0: PC6 - Input floating (прерывание от CC1101) === */ 
  GPIOC->CRL &= ~(0xF << 24);  
  GPIOC->CRL |=  (0x4 << 24);  
  /* PC6 - Input floating */  
  /* Включить прерывание EXTI6 для PC6 */ 
  RCC->APB2ENR |= (1 << 0);    
  /* AFIO clock */  
  /* AFIO->EXTICR[1] |= (0x2 << 8); */ /* PC6 -> EXTI6 (закомментировано, раскомментировать при настройке EXTI) */
#endif
}

