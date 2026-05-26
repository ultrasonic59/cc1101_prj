/********************************************************************************* 
* at32f415xx.h * CMSIS-style Peripheral Access Layer for AT32F415RCT7 (Artery) 
* Generated for IAR 9.30 + FreeRTOS 
* ******************************************************************************/

#ifndef __AT32F415XX_H__
#define __AT32F415XX_H__

#ifdef __cplusplus
extern "C" { 
#endif
/* Cortex-M4 core */
#include <stdint.h>
#define __CM4_REV                 0x0001
#define __MPU_PRESENT             0
#define __NVIC_PRIO_BITS          4
#define __Vendor_SysTickConfig    0
#define __FPU_PRESENT             0
/* Vector table */
extern uint32_t __Vectors[];
typedef void (*pFunc)(void);
/* Base addresses */
#define FLASH_BASE            (0x08000000UL)
#define SRAM_BASE             (0x20000000UL)
#define PERIPH_BASE           (0x40000000UL)
#define APB1PERIPH_BASE       (PERIPH_BASE)
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x00010000UL)
#define AHBPERIPH_BASE        (PERIPH_BASE + 0x00020000UL)
/* APB1 peripherals */
#define TIM2_BASE             (APB1PERIPH_BASE + 0x0000)
#define TIM3_BASE             (APB1PERIPH_BASE + 0x0400)
#define TIM4_BASE             (APB1PERIPH_BASE + 0x0800)
#define TIM5_BASE             (APB1PERIPH_BASE + 0x0C00)
#define TIM6_BASE             (APB1PERIPH_BASE + 0x1000)
#define TIM7_BASE             (APB1PERIPH_BASE + 0x1400)
#define TIM12_BASE            (APB1PERIPH_BASE + 0x1800)
#define TIM13_BASE            (APB1PERIPH_BASE + 0x1C00)
#define TIM14_BASE            (APB1PERIPH_BASE + 0x2000)
#define RTC_BASE              (APB1PERIPH_BASE + 0x2800)
#define WWDG_BASE             (APB1PERIPH_BASE + 0x2C00)
#define IWDG_BASE             (APB1PERIPH_BASE + 0x3000)
#define SPI2_BASE             (APB1PERIPH_BASE + 0x3800)
#define SPI3_BASE             (APB1PERIPH_BASE + 0x3C00)
#define USART2_BASE           (APB1PERIPH_BASE + 0x4400)
#define USART3_BASE           (APB1PERIPH_BASE + 0x4800)
#define UART4_BASE            (APB1PERIPH_BASE + 0x4C00)
#define UART5_BASE            (APB1PERIPH_BASE + 0x5000)
#define I2C1_BASE             (APB1PERIPH_BASE + 0x5400)
#define I2C2_BASE             (APB1PERIPH_BASE + 0x5800)
#define CAN1_BASE             (APB1PERIPH_BASE + 0x6400)
#define CAN2_BASE             (APB1PERIPH_BASE + 0x6800)
#define BKP_BASE              (APB1PERIPH_BASE + 0x6C00)
#define PWR_BASE              (APB1PERIPH_BASE + 0x7000)
#define DAC_BASE              (APB1PERIPH_BASE + 0x7400)
/* APB2 peripherals */
#define TIM1_BASE             (APB2PERIPH_BASE + 0x0000)
#define TIM8_BASE             (APB2PERIPH_BASE + 0x0400)
#define USART1_BASE           (APB2PERIPH_BASE + 0x1000)
#define USART6_BASE           (APB2PERIPH_BASE + 0x1400)
#define ADC1_BASE             (APB2PERIPH_BASE + 0x2000)
#define ADC2_BASE             (APB2PERIPH_BASE + 0x2400)
#define ADC3_BASE             (APB2PERIPH_BASE + 0x2800)
#define SDIO_BASE             (APB2PERIPH_BASE + 0x2C00)
#define SPI1_BASE             (APB2PERIPH_BASE + 0x3000)
#define SPI4_BASE             (APB2PERIPH_BASE + 0x3400)
#define SYSCFG_BASE           (APB2PERIPH_BASE + 0x3800)
#define EXTI_BASE             (APB2PERIPH_BASE + 0x3C00)
#define TIM9_BASE             (APB2PERIPH_BASE + 0x4000)
#define TIM10_BASE            (APB2PERIPH_BASE + 0x4400)
#define TIM11_BASE            (APB2PERIPH_BASE + 0x4800)
/* AHB peripherals */
#define DMA1_BASE             (AHBPERIPH_BASE + 0x0000)
#define DMA2_BASE             (AHBPERIPH_BASE + 0x0400)
#define RCC_BASE              (AHBPERIPH_BASE + 0x1000)
#define FLASH_R_BASE          (AHBPERIPH_BASE + 0x2000)
#define CRC_BASE              (AHBPERIPH_BASE + 0x3000)
#define GPIOA_BASE            (AHBPERIPH_BASE + 0x8000)
#define GPIOB_BASE            (AHBPERIPH_BASE + 0x8400)
#define GPIOC_BASE            (AHBPERIPH_BASE + 0x8800)
#define GPIOD_BASE            (AHBPERIPH_BASE + 0x8C00)
#define GPIOE_BASE            (AHBPERIPH_BASE + 0x9000)
#define GPIOF_BASE            (AHBPERIPH_BASE + 0x9400)
#define GPIOG_BASE            (AHBPERIPH_BASE + 0x9800)
#define GPIOH_BASE            (AHBPERIPH_BASE + 0x9C00)
/* Core peripherals */
#define SCS_BASE              (0xE000E000UL)
#define SysTick_BASE          (SCS_BASE + 0x0010)
#define NVIC_BASE             (SCS_BASE + 0x0100)
#define SCB_BASE              (SCS_BASE + 0x0D00)
/* Register structures */
typedef struct{  
  volatile uint32_t CR;  
  volatile uint32_t CFGR;  
  volatile uint32_t CIR;  
  volatile uint32_t APB2RSTR;  
  volatile uint32_t APB1RSTR;  
  volatile uint32_t AHBENR;  
  volatile uint32_t APB2ENR;  
  volatile uint32_t APB1ENR;  
  volatile uint32_t BDCR;  
  volatile uint32_t CSR;  
  volatile uint32_t AHBSTR;  
  volatile uint32_t CFGR2;
} RCC_TypeDef;
typedef struct{  
  volatile uint32_t ACR;  
  volatile uint32_t KEYR;  
  volatile uint32_t OPTKEYR;  
  volatile uint32_t SR;  
  volatile uint32_t CR; 
  volatile uint32_t AR;  
  volatile uint32_t RESERVED; 
  volatile uint32_t OBR; 
  volatile uint32_t WRPR;
} FLASH_TypeDef;
typedef struct gpio_s_{
  volatile uint32_t CRL;
  volatile uint32_t CRH;
  volatile uint32_t IDR;
  volatile uint32_t ODR;
  volatile uint32_t BSRR;
  volatile uint32_t BRR;
  volatile uint32_t LCKR;
} GPIO_TypeDef;

typedef struct{  
  volatile uint32_t CR1;  
  volatile uint32_t CR2;  
  volatile uint32_t SR;  
  volatile uint32_t DR;  
  volatile uint32_t CRCPR;  
  volatile uint32_t RXCRCR;  
  volatile uint32_t TXCRCR; 
  volatile uint32_t I2SCFGR;  
  volatile uint32_t I2SPR;
} SPI_TypeDef;
typedef struct{  
  volatile uint32_t SR;  
  volatile uint32_t DR;  
  volatile uint32_t BRR;  
  volatile uint32_t CR1;  
  volatile uint32_t CR2;  
  volatile uint32_t CR3;  
  volatile uint32_t GTPR;
} USART_TypeDef;
typedef struct{ 
  volatile uint32_t CTRL; 
  volatile uint32_t LOAD; 
  volatile uint32_t VAL;
  volatile uint32_t CALIB;
} SysTick_TypeDef;
typedef struct{  
  volatile uint32_t CPUID;  
  volatile uint32_t ICSR; 
  volatile uint32_t VTOR; 
  volatile uint32_t AIRCR; 
  volatile uint32_t SCR; 
  volatile uint32_t CCR;  
  volatile uint32_t SHPR[3];  
  volatile uint32_t SHCSR;  
  volatile uint32_t CFSR;  
  volatile uint32_t HFSR;  
  volatile uint32_t DFSR;  
  volatile uint32_t MMFAR;  
  volatile uint32_t BFAR;  
  volatile uint32_t AFSR;
} SCB_TypeDef;
/* Peripheral declarations */
#define RCC               ((RCC_TypeDef *) RCC_BASE)
#define FLASH             ((FLASH_TypeDef *) FLASH_R_BASE)
#define GPIOA             ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB             ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC             ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD             ((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOE             ((GPIO_TypeDef *) GPIOE_BASE)
#define GPIOF             ((GPIO_TypeDef *) GPIOF_BASE)
#define GPIOG             ((GPIO_TypeDef *) GPIOG_BASE)
#define GPIOH             ((GPIO_TypeDef *) GPIOH_BASE)
#define SPI1                ((SPI_TypeDef *) SPI1_BASE)
#define SPI2                ((SPI_TypeDef *) SPI2_BASE)
#define USART1              ((USART_TypeDef *) USART1_BASE)
#define USART2              ((USART_TypeDef *) USART2_BASE)
#define SysTick           ((SysTick_TypeDef *) SysTick_BASE)
#define SCB               ((SCB_TypeDef *) SCB_BASE)/* RCC bits */
#define RCC_CR_HSION                     (1UL << 0)
#define RCC_CR_HSIRDY                    (1UL << 1)
#define RCC_CR_HSEON                     (1UL << 16)
#define RCC_CR_HSERDY                    (1UL << 17)
#define RCC_CR_HSEBYP                    (1UL << 18)
#define RCC_CR_CSSON                     (1UL << 19) // 

#define RCC_CR_PLLON                     (1UL << 24)
#define RCC_CR_PLLRDY                    (1UL << 25)
#define RCC_CFGR_SW                      (3UL << 0)
#define RCC_CFGR_SW_PLL                  (2UL << 0)
#define RCC_CFGR_SWS                     (3UL << 2)
#define RCC_CFGR_SWS_PLL                 (2UL << 2)
#define RCC_CFGR_HPRE_DIV1               (0UL << 4)
#define RCC_CFGR_PPRE1_DIV2              (4UL << 8)
#define RCC_CFGR_PPRE2_DIV1              (0UL << 11)
#define RCC_CFGR_PLLSRC_HSE              (1UL << 16)
#define RCC_CFGR_PLLMUL_6                (4UL << 18)
#define RCC_APB2ENR_IOPAEN               (1UL << 2)
#define RCC_APB2ENR_IOPBEN               (1UL << 3)
#define RCC_APB2ENR_IOPCEN               (1UL << 4)
#define RCC_APB1ENR_USART2EN             (1UL << 17)
#define RCC_APB1ENR_SPI2EN               (1UL << 14)
#define RCC_AHBENR_DMA1EN                (1UL << 0)/* Flash bits */
#define FLASH_ACR_PRFTBE                 (1UL << 4)
#define FLASH_ACR_LATENCY_1              (1UL << 0)/* NVIC */
#define NVIC_EnableIRQ(IRQn)             (NVIC->ISER[((uint32_t)(IRQn) >> 5)] = (1UL << ((uint32_t)(IRQn) & 0x1F)))
#define NVIC_SetPriority(IRQn, priority) (NVIC->IP[(uint32_t)(IRQn)] = (uint8_t)((priority << 4) & 0xFF))
typedef enum{ 
  NonMaskableInt_IRQn         = -14,  
  MemoryManagement_IRQn       = -12,  
  BusFault_IRQn               = -11, 
  UsageFault_IRQn             = -10,  
  SVCall_IRQn                 = -5,  
  DebugMonitor_IRQn           = -4,  
  PendSV_IRQn                 = -2, 
  SysTick_IRQn                = -1, 
  WWDG_IRQn                   = 0, 
  PVD_IRQn                    = 1,  
  TAMPER_IRQn                 = 2,  
  RTC_IRQn                    = 3, 
  FLASH_IRQn                  = 4, 
  RCC_IRQn                    = 5,
  EXTI0_IRQn                  = 6,
  EXTI1_IRQn                  = 7, 
  EXTI2_IRQn                  = 8, 
  EXTI3_IRQn                  = 9, 
  EXTI4_IRQn                  = 10,
  DMA1_Channel1_IRQn          = 11,
  DMA1_Channel2_IRQn          = 12, 
  DMA1_Channel3_IRQn          = 13, 
  DMA1_Channel4_IRQn          = 14,
  DMA1_Channel5_IRQn          = 15, 
  DMA1_Channel6_IRQn          = 16, 
  DMA1_Channel7_IRQn          = 17,
  ADC1_2_IRQn                 = 18,  
  USB_HP_CAN1_TX_IRQn         = 19, 
  USB_LP_CAN1_RX0_IRQn        = 20, 
  CAN1_RX1_IRQn               = 21, 
  CAN1_SCE_IRQn               = 22, 
  EXTI9_5_IRQn                = 23, 
  TIM1_BRK_TIM9_IRQn          = 24,
  TIM1_UP_TIM10_IRQn          = 25, 
  TIM1_TRG_COM_TIM11_IRQn     = 26,
  TIM1_CC_IRQn                = 27, 
  TIM2_IRQn                   = 28,
  TIM3_IRQn                   = 29,
  TIM4_IRQn                   = 30, 
  I2C1_EV_IRQn                = 31,  
  I2C1_ER_IRQn                = 32, 
  I2C2_EV_IRQn                = 33, 
  I2C2_ER_IRQn                = 34, 
  SPI1_IRQn                   = 35, 
  SPI2_IRQn                   = 36, 
  USART1_IRQn                 = 37, 
  USART2_IRQn                 = 38, 
  USART3_IRQn                 = 39, 
  EXTI15_10_IRQn              = 40, 
  RTCAlarm_IRQn               = 41, 
  USBWakeUp_IRQn              = 42, 
  TIM8_BRK_IRQn               = 43, 
  TIM8_UP_IRQn                = 44, 
  TIM8_TRG_COM_IRQn           = 45, 
  TIM8_CC_IRQn                = 46, 
  ADC3_IRQn                   = 47, 
  SDIO_IRQn                   = 49, 
  TIM5_IRQn                   = 50, 
  SPI3_IRQn                   = 51, 
  UART4_IRQn                  = 52, 
  UART5_IRQn                  = 53, 
  TIM6_IRQn                   = 54, 
  TIM7_IRQn                   = 55, 
  DMA2_Channel1_IRQn          = 56,  
  DMA2_Channel2_IRQn          = 57,  
  DMA2_Channel3_IRQn          = 58,  
  DMA2_Channel4_5_IRQn        = 59
} IRQn_Type;
/* NVIC structure */
typedef struct{  
  volatile uint32_t ISER[8];  
  uint32_t RESERVED0[24];  
  volatile uint32_t ICER[8]; 
  uint32_t RSERVED1[24];
  volatile uint32_t ISPR[8]; 
  uint32_t RESERVED2[24]; 
  volatile uint32_t ICPR[8]; 
  uint32_t RESERVED3[24];
  volatile uint32_t IABR[8]; 
  uint32_t RESERVED4[56]; 
  volatile uint8_t  IP[240]; 
  uint32_t RESERVED5[644]; 
  volatile uint32_t STIR;
} NVIC_TypeDef;
#define NVIC                ((NVIC_TypeDef *) NVIC_BASE)
/* SystemCoreClock */
 extern uint32_t SystemCoreClock;
 void SystemCoreClockUpdate(void);
#ifdef __cplusplus
}
#endif
#endif /* __AT32F415XX_H */