#include <string.h>
#include "rtos.h"

#include "bat_brd.h"
#include "printk.h"
#include "at32f415.h"
#include "uart.h"

///__IO uint16_t adc1_ordinary_valuetab[NUM_ADC_IN] = {0};
volatile uint32_t msTicks;
////uint32_t stat_contr = 0;

void init_gpio(void)
{
  
////RCC_AHBPeriphClockCmd(XEN_PIN_RCC_AHB,ENABLE);
gpio_init_type gpio_init_struct;
crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK, TRUE);
crm_periph_clock_enable(CRM_GPIOF_PERIPH_CLOCK, TRUE);
crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);
gpio_pin_remap_config(SWJTAG_GMUX_010,TRUE);///for pa15,pb3,pb4
////=============RLED========================
gpio_default_para_init(&gpio_init_struct);
gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_pins           = RLED_PIN;
gpio_init(RLED_GPIO, &gpio_init_struct);
////============= GLED ========================
gpio_default_para_init(&gpio_init_struct);
gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_pins           = GLED_PIN;
gpio_init(GLED_GPIO, &gpio_init_struct);
////============== GDO2 ======================================================  
gpio_default_para_init(&gpio_init_struct);
gpio_init_struct.gpio_mode           = GPIO_MODE_INPUT;
gpio_init_struct.gpio_pins           = GDO2_PIN;
gpio_init(GDO2_GPIO, &gpio_init_struct);
////============== GDO0 ======================================================  
gpio_default_para_init(&gpio_init_struct);
gpio_init_struct.gpio_mode           = GPIO_MODE_INPUT;
gpio_init_struct.gpio_pins           = GDO0_PIN;
gpio_init(GDO0_GPIO, &gpio_init_struct);
////============== KEY======================================================  
gpio_default_para_init(&gpio_init_struct);
gpio_init_struct.gpio_mode           = GPIO_MODE_INPUT;
gpio_init_struct.gpio_pins           = KEY_PIN;
gpio_init(KEY_GPIO, &gpio_init_struct);
////=========== DBG_UART =================================================== 

///gpio_pin_remap_config(UART_DBG_GMUX,TRUE);
gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_pins           = UART_DBG_TX_PIN;
gpio_init(UART_DBG_TX_GPIO, &gpio_init_struct);
gpio_init_struct.gpio_mode           = GPIO_MODE_INPUT;
gpio_init_struct.gpio_pins           = UART_DBG_RX_PIN;
gpio_init(UART_DBG_RX_GPIO, &gpio_init_struct);
////=========== UART_PC =================================================== 
///gpio_pin_remap_config(UART_DBG_GMUX,TRUE);
gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_pins           = UART_PC_TX_PIN;
gpio_init(UART_PC_TX_GPIO, &gpio_init_struct);
gpio_init_struct.gpio_mode           = GPIO_MODE_INPUT;
gpio_init_struct.gpio_pins           = UART_PC_RX_PIN;
gpio_init(UART_PC_RX_GPIO, &gpio_init_struct);


#if 0
////=============tst0==================================
gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_pins           = TST0_PIN;
gpio_init(TST0_GPIO, &gpio_init_struct);
////=============tst1==================================
gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_pins           = TST1_PIN;
gpio_init(TST1_GPIO, &gpio_init_struct);
////=============tst2==================================
gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_pins           = TST2_PIN;
gpio_init(TST2_GPIO, &gpio_init_struct);
////=============tst3==================================
gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_pins           = TST3_PIN;
gpio_init(TST3_GPIO, &gpio_init_struct);
////=============tst4==================================
gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_pins           = TST4_PIN;
gpio_init(TST4_GPIO, &gpio_init_struct);
////=============tst5==================================
gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_pins           = TST5_PIN;
gpio_init(TST5_GPIO, &gpio_init_struct);
////=============tst6==================================
gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_pins           = TST6_PIN;
gpio_init(TST6_GPIO, &gpio_init_struct);
////=============tst7==================================
gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_pins           = TST7_PIN;
gpio_init(TST7_GPIO, &gpio_init_struct);

#endif
}
////======================================================
////=====led ================
void put_led(uint8_t i_dat)
{
if(i_dat&0x1)
  RLED_GPIO->scr= RLED_PIN;
else
  RLED_GPIO->clr= RLED_PIN;
if(i_dat&(0x1<<1))
  GLED_GPIO->scr= GLED_PIN;
else
  GLED_GPIO->clr= GLED_PIN;
}
#if 0
void LED_UART_RX_On(void)  { 
 put_led(0x1) ;
 /// GPIOC->ODR &= ~(1 << 13); 
}
void LED_UART_RX_Off(void) { 
 put_led(0x0) ;
///  GPIOC->ODR |=  (1 << 13);
}

void LED_RF_TX_On(void)    {
 put_led(0x2) ;
///  GPIOC->ODR &= ~(1 << 14); 
}
void LED_RF_TX_Off(void)   { 
///  GPIOC->ODR |=  (1 << 14); 
 put_led(0x0) ;

}
#endif
void spi_rf_init(void){
  spi_init_type spi_init_struct;
  gpio_init_type gpio_initstructure;
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
  gpio_default_para_init(&gpio_initstructure);
  /* sck pin */
  gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
  gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_initstructure.gpio_pull           = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_mode         = GPIO_MODE_MUX;
  gpio_initstructure.gpio_pins           = SCK_RF_PIN;
  gpio_init(SCK_RF_GPIO, &gpio_initstructure);

  /* miso pin — без pull, CC1101 сам драйвит линию */
  gpio_initstructure.gpio_pull           = GPIO_PULL_NONE;
  gpio_initstructure.gpio_mode         = GPIO_MODE_INPUT;
  gpio_initstructure.gpio_pins           = MISO_RF_PIN;
  gpio_init(MISO_RF_GPIO, &gpio_initstructure);

  /* mosi pin */
  gpio_initstructure.gpio_pull           = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_mode         = GPIO_MODE_MUX;
  gpio_initstructure.gpio_pins           = MOSI_RF_PIN;
  gpio_init(MOSI_RF_GPIO, &gpio_initstructure);
   /* ncs pin */
  gpio_default_para_init(&gpio_initstructure);
  gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
  gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
  gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_initstructure.gpio_pins           = NCS_RF_PIN;
  gpio_init(NCS_RF_GPIO, &gpio_initstructure);
  gpio_bits_set(NCS_RF_GPIO, NCS_RF_PIN);

  
  crm_periph_clock_enable(CRM_SPI2_PERIPH_CLOCK, TRUE);
  spi_default_para_init(&spi_init_struct);
  spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
  spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
  spi_init_struct.mclk_freq_division = SPI_RF_MCLK_DIV;
  spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
  spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
  spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
  spi_init_struct.clock_phase = SPI_CLOCK_PHASE_1EDGE;
  spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
  spi_init(SPI2, &spi_init_struct);

  spi_enable(SPI2, TRUE);
  
}

void spi_rf_configure(uint8_t msb_first, uint8_t cpha_2edge, uint8_t mclk_div)
{
  spi_init_type spi_init_struct;

  spi_enable(SPI2, FALSE);
  spi_default_para_init(&spi_init_struct);
  spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
  spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
  spi_init_struct.mclk_freq_division = (spi_mclk_freq_div_type)mclk_div;
  spi_init_struct.first_bit_transmission =
      msb_first ? SPI_FIRST_BIT_MSB : SPI_FIRST_BIT_LSB;
  spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
  spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
  spi_init_struct.clock_phase =
      cpha_2edge ? SPI_CLOCK_PHASE_2EDGE : SPI_CLOCK_PHASE_1EDGE;
  spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
  spi_init(SPI2, &spi_init_struct);
  spi_enable(SPI2, TRUE);
}
#if 0
////=====tst0 ================

void set_tst0(uint8_t i_dat)
{
if(i_dat&0x1)
  TST0_GPIO->scr= TST0_PIN;
else
  TST0_GPIO->clr= TST0_PIN;
}
void set_tst1(uint8_t i_dat)
{
if(i_dat&0x1)
  TST1_GPIO->scr= TST1_PIN;
else
  TST1_GPIO->clr= TST1_PIN;
}
void set_tst2(uint8_t i_dat)
{
if(i_dat&0x1)
  TST2_GPIO->scr= TST2_PIN;
else
  TST2_GPIO->clr= TST2_PIN;
}
void set_tst3(uint8_t i_dat)
{
if(i_dat&0x1)
  TST3_GPIO->scr= TST3_PIN;
else
  TST3_GPIO->clr= TST3_PIN;
}
void set_tst4(uint8_t i_dat)
{
if(i_dat&0x1)
  TST4_GPIO->scr= TST4_PIN;
else
  TST4_GPIO->clr= TST4_PIN;
}
void set_tst5(uint8_t i_dat)
{
if(i_dat&0x1)
  TST5_GPIO->scr= TST5_PIN;
else
  TST5_GPIO->clr= TST5_PIN;
}
void set_tst6(uint8_t i_dat)
{
if(i_dat&0x1)
  TST6_GPIO->scr= TST6_PIN;
else
  TST6_GPIO->clr= TST6_PIN;
}
void set_tst7(uint8_t i_dat)
{
if(i_dat&0x1)
  TST7_GPIO->scr= TST7_PIN;
else
  TST7_GPIO->clr= TST7_PIN;
}
#endif
uint8_t  get_gdo0(void)
{
if(gpio_input_data_bit_read(GDO0_GPIO, GDO0_PIN) ) 
   return  0;
else
  return 1;
}
uint8_t  get_gdo2(void)
{
if(gpio_input_data_bit_read(GDO2_GPIO, GDO2_PIN) ) 
   return  0;
else
  return 1;
}
uint8_t  get_key(void)
{
if(gpio_input_data_bit_read(KEY_GPIO, KEY_PIN) ) 
   return  0;
else
  return 1;
}



/* delay variable */
static __IO uint32_t fac_us;
static __IO uint32_t fac_ms;


void delay_init(void)
{
  systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
  fac_us = system_core_clock / (1000000U);
  fac_ms = fac_us * (1000U);
}

void delay_us(uint32_t nus)
{
    volatile uint32_t i;
    uint32_t loops = nus * fac_us;

    if (loops == 0U) {
        loops = 1U;
    }
    for (i = 0; i < loops; i++) {
        __NOP();
    }
}

void delay_ms(uint16_t nms)
{
    while (nms > 0U) {
        uint16_t chunk = (nms > 50U) ? 50U : nms;
        delay_us((uint32_t)chunk * 1000U);
        nms = (uint16_t)(nms - chunk);
    }
}

void hw_board_init(void)
{
init_gpio();
system_clock_config();
UART_DBG_Init(); 
spi_rf_init();
///init_def_uart();
///dma_uart_config();

delay_init();
///init_motor();
}

////============================================
////=========================================
void print_dbg(char *name,int32_t val)
{
  static int32_t prev_val=0;
if (prev_val!= val)
  {
  prev_val= val;
  printk("\n\r[%s][%x:%d]",name,val,val);

  }
}

///================================

///====================================