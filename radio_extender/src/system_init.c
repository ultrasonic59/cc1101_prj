/*******************************************************************************
 * system_init.c
 * Базовая инициализация системы AT32F415RCT7
 * HSE = 8 MHz, PLL*6 = 48 MHz, Flash 1 WS
 ******************************************************************************/

#include "at32f415xx.h"

/* Векторная таблица */
void (* const g_pfnVectors[])(void) __attribute__ ((section(".isr_vector")));

void SystemInit(void)
{
    /* 1. Сброс RCC */
    RCC->CR |= RCC_CR_HSION;
    RCC->CFGR = 0x00000000;
    RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_CSSON | RCC_CR_HSEON);
    RCC->CR &= ~RCC_CR_HSEBYP;
    RCC->CIR = 0x00000000;

    /* 2. Включить HSE */
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & RCC_CR_HSERDY) == 0);

    /* 3. PLL: HSE / 1 * 6 = 48 MHz */
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMUL_6;

    /* 4. Flash: 1 WS для 48 MHz */
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_1;

    /* 5. AHB = SYSCLK, APB1 = SYSCLK/2, APB2 = SYSCLK */
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PPRE2_DIV1;

    /* 6. Включить PLL */
    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0);

    /* 7. SYSCLK = PLL */
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS_PLL) == 0);

    /* 8. Обновить SystemCoreClock */
    SystemCoreClock = 48000000;
}

/* Слабые обработчики по умолчанию */
void Default_Handler(void) { while(1); }

/* Обработчики */
void NMI_Handler(void)        __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__ ((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__ ((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__ ((weak, alias("Default_Handler")));
