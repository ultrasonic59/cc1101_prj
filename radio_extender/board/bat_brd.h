#ifndef __BAT_BRD_H__
#define __BAT_BRD_H__

#include <stdint.h>
#include "at32f415.h"
///#include "uarts.h"
///#include "otter_types.h"
///#include "circ_buf.h"

#define LED_GREEN  0x2
#define LED_RED    0x1
#define LED_OFF    0x0

#define USB_TASK_STACK_SIZE		2048

/* USB_VBUS_IGNORE — только в src/usb_conf.h (PA9 = USART1 TX, rev C). */

#define APP_PRIORITY	                4
#define USB_PRIORITY	                4
#define TST_TASK_STACK_SIZE		512
#define TST_TASK_PRIORITY		5

#include "at32f415_clock.h"
#include "at32f415_spi.h"

/* CC1101 SPI2: MSB, CPOL=Low, CPHA=1Edge, prescaler 32 (≈2.25 МГц при APB1=72 МГц) */
#define SPI_RF_MCLK_DIV         SPI_MCLK_DIV_128 ///SPI_MCLK_DIV_32

#define DBG_BR      115200
////=============RLED==================================
#define RLED_PIN	       GPIO_PINS_12
#define RLED_GPIO	  	GPIOC
////=============GLED==================================
#define GLED_PIN	       GPIO_PINS_15
#define GLED_GPIO	  	GPIOA
////=============GDO2==================================
#define GDO2_PIN	        GPIO_PINS_0
#define GDO2_GPIO	        GPIOB
////=============GDO0==================================
#define GDO0_PIN	        GPIO_PINS_1
#define GDO0_GPIO	        GPIOB
////=============SPI_RF==================================
////=============MISO_RF==================================
#define MISO_RF_PIN	       GPIO_PINS_14
#define MISO_RF_GPIO	  	GPIOB
////=============MOSI_RF==================================
#define MOSI_RF_PIN	       GPIO_PINS_15
#define MOSI_RF_GPIO	  	GPIOB
////=============SCK_RF==================================
#define SCK_RF_PIN	       GPIO_PINS_13
#define SCK_RF_GPIO	  	GPIOB
////=============NCS_RF==================================
#define NCS_RF_PIN	       GPIO_PINS_12
#define NCS_RF_GPIO	  	GPIOB
////=============KEY==================================
#define KEY_PIN	                GPIO_PINS_8
#define KEY_GPIO	  	GPIOC

////=========== UART_PC_TX ===============================
#define  UART_PC_TX_PIN	    	GPIO_PINS_9
#define  UART_PC_TX_GPIO          GPIOA
////=========== UART_PC_RX ===============================
#define  UART_PC_RX_PIN	    	GPIO_PINS_10
#define  UART_PC_RX_GPIO          GPIOA
#define UART_PC                 USART1
#define  UART_PC_irq_hnd     USART1_IRQHandler
#define  UART_PC_IRQn        USART1_IRQn
#define  UART_PC_PERIPH_CLOCK    CRM_USART1_PERIPH_CLOCK 
///#define  UART_PC_DMA              DMA1_CHANNEL2

////=========== DBG_TX ===============================
#define UART_DBG_TX_PIN	    	GPIO_PINS_2
#define UART_DBG_TX_GPIO        GPIOA
////=========== DBG_RX ===============================
#define UART_DBG_RX_PIN	    	GPIO_PINS_3
#define UART_DBG_RX_GPIO        GPIOA
////============================================
#define UART_DBG                USART2
#define UART_DBG_irq_hnd        UART2_IRQHandler
#define UART_DBG_PERIPH_CLOCK   CRM_USART2_PERIPH_CLOCK 


#include "tx_api.h"
#define msleep(ticks)  tx_thread_sleep(ticks)
extern volatile uint32_t msTicks;

///extern dbg_dat_req_t cur_dbg_dat;
void delay_init(void);
void delay_us(uint32_t us);
void delay_ms(uint16_t ms);

extern void  hw_board_init(void);
extern void put_led(uint8_t led);
extern uint8_t get_gdo0(void);
extern uint8_t get_gdo2(void);
extern void spi_rf_configure(uint8_t msb_first, uint8_t cpha_2edge, uint8_t mclk_div);

extern void LED_UART_RX_On(void);
extern void LED_UART_RX_Off(void) ;
extern void LED_RF_TX_On(void) ; 
extern void LED_RF_TX_Off(void);

#if 0
////extern uint8_t cur_status;
extern uint16_t  get_inp(void);
///extern contr_stat_t cur_stat;
extern uint8_t get_contr_dat(uint8_t* buf);
extern int8_t  put_req_addr(uint8_t *buf);
extern int8_t put_contr(uint8_t *buf);
extern void put_out0(uint8_t i_dat);
extern void put_out1(uint8_t i_dat);
extern void put_out2(uint8_t i_dat);
extern void put_out3(uint8_t i_dat);
extern void put_out4(uint8_t i_dat);
extern void put_out5(uint8_t i_dat);
extern void put_out6(uint8_t i_dat);
extern void put_out7(uint8_t i_dat);
extern void put_out8(uint8_t i_dat);
extern void put_out9(uint8_t i_dat);
extern void put_out10(uint8_t i_dat);
extern void put_out11(uint8_t i_dat);
extern void put_out12(uint8_t i_dat);
extern void put_out13(uint8_t i_dat);
extern void put_out14(uint8_t i_dat);
extern void put_out15(uint8_t i_dat);
extern void put_out_n(uint16_t ttst);

extern void set_out_n(uint16_t ttst);
extern void clr_out_n(uint16_t ttst);
extern void togle_out_n(uint16_t ttst);
extern void set_tst0(uint8_t i_dat);
extern void set_tst1(uint8_t i_dat);
extern void set_tst2(uint8_t i_dat);
extern void set_tst3(uint8_t i_dat);
extern void set_tst4(uint8_t i_dat);
extern void set_tst5(uint8_t i_dat);
extern void set_tst6(uint8_t i_dat);
extern void set_tst7(uint8_t i_dat);

extern void print_dbg(char *name,int32_t val);

extern uint8_t send_485ack(uint8_t *buff, uint8_t len);
#endif
////=============================================
#endif ////__OTTER_BRD_H___



	
