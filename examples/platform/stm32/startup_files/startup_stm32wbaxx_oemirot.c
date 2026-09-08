/*
 * Copyright (c) 2009-2020 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * This file is derivative of CMSIS V5.6.0 startup_ARMv81MML.c
 * Git SHA:
 */

#include "stm32_hal.h"
/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void (*pFunc)(void);

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern uint32_t __INITIAL_SP;
extern uint32_t __STACK_LIMIT;

extern void __PROGRAM_START(void) __NO_RETURN;

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
void Reset_Handler(void) __NO_RETURN;

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
#define DEFAULT_IRQ_HANDLER(handler_name)                                                                                          \
    void handler_name(void);                                                                                                       \
    __WEAK void handler_name(void)                                                                                                 \
    {                                                                                                                              \
        while (1)                                                                                                                  \
            ;                                                                                                                      \
    }

/* Exceptions */
DEFAULT_IRQ_HANDLER(NMI_Handler)
DEFAULT_IRQ_HANDLER(HardFault_Handler)
DEFAULT_IRQ_HANDLER(MemManage_Handler)
DEFAULT_IRQ_HANDLER(BusFault_Handler)
DEFAULT_IRQ_HANDLER(UsageFault_Handler)
DEFAULT_IRQ_HANDLER(SecureFault_Handler)
DEFAULT_IRQ_HANDLER(SVC_Handler)
DEFAULT_IRQ_HANDLER(DebugMon_Handler)
DEFAULT_IRQ_HANDLER(PendSV_Handler)
DEFAULT_IRQ_HANDLER(SysTick_Handler)

DEFAULT_IRQ_HANDLER(WWDG_IRQHandler)
DEFAULT_IRQ_HANDLER(PVD_AVD_IRQHandler)
DEFAULT_IRQ_HANDLER(RTC_IRQHandler)
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx)
DEFAULT_IRQ_HANDLER(RTC_IRQHandler_S)
#endif
DEFAULT_IRQ_HANDLER(TAMP_IRQHandler)
DEFAULT_IRQ_HANDLER(RAMCFG_IRQHandler)
DEFAULT_IRQ_HANDLER(FLASH_IRQHandler)
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
DEFAULT_IRQ_HANDLER(FLASH_IRQHandler_S)
DEFAULT_IRQ_HANDLER(GTZC_IRQHandler)
#endif
DEFAULT_IRQ_HANDLER(RCC_IRQHandler)
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx)
DEFAULT_IRQ_HANDLER(RCC_IRQHandler_S)
#endif
DEFAULT_IRQ_HANDLER(EXTI0_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI1_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI2_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI3_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI4_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI5_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI6_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI7_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI8_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI9_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI10_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI11_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI12_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI13_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI14_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI15_IRQHandler)
DEFAULT_IRQ_HANDLER(IWDG_IRQHandler)
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
DEFAULT_IRQ_HANDLER(SAES_IRQHandler)
#endif
DEFAULT_IRQ_HANDLER(GPDMA1_Channel0_IRQHandler)
DEFAULT_IRQ_HANDLER(GPDMA1_Channel1_IRQHandler)
DEFAULT_IRQ_HANDLER(GPDMA1_Channel2_IRQHandler)
DEFAULT_IRQ_HANDLER(GPDMA1_Channel3_IRQHandler)
DEFAULT_IRQ_HANDLER(GPDMA1_Channel4_IRQHandler)
DEFAULT_IRQ_HANDLER(GPDMA1_Channel5_IRQHandler)
DEFAULT_IRQ_HANDLER(GPDMA1_Channel6_IRQHandler)
DEFAULT_IRQ_HANDLER(GPDMA1_Channel7_IRQHandler)
DEFAULT_IRQ_HANDLER(TIM1_BRK_IRQHandler)
DEFAULT_IRQ_HANDLER(TIM1_UP_IRQHandler)
DEFAULT_IRQ_HANDLER(TIM1_TRG_COM_IRQHandler)
DEFAULT_IRQ_HANDLER(TIM1_CC_IRQHandler)
DEFAULT_IRQ_HANDLER(TIM2_IRQHandler)
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
DEFAULT_IRQ_HANDLER(TIM3_IRQHandler)
DEFAULT_IRQ_HANDLER(I2C1_EV_IRQHandler)
DEFAULT_IRQ_HANDLER(I2C1_ER_IRQHandler)
DEFAULT_IRQ_HANDLER(SPI1_IRQHandler)
#endif
DEFAULT_IRQ_HANDLER(USART1_IRQHandler)
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
DEFAULT_IRQ_HANDLER(USART2_IRQHandler)
#endif
DEFAULT_IRQ_HANDLER(LPUART1_IRQHandler)
DEFAULT_IRQ_HANDLER(LPTIM1_IRQHandler)
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
DEFAULT_IRQ_HANDLER(LPTIM2_IRQHandler)
#endif
DEFAULT_IRQ_HANDLER(TIM16_IRQHandler)
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
DEFAULT_IRQ_HANDLER(TIM17_IRQHandler)
#endif
#if defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
DEFAULT_IRQ_HANDLER(COMP_IRQHandler)
#endif
DEFAULT_IRQ_HANDLER(I2C3_EV_IRQHandler)
DEFAULT_IRQ_HANDLER(I2C3_ER_IRQHandler)
#if defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
DEFAULT_IRQ_HANDLER(SAI1_IRQHandler)
#endif
DEFAULT_IRQ_HANDLER(TSC_IRQHandler)
DEFAULT_IRQ_HANDLER(AES_IRQHandler)
DEFAULT_IRQ_HANDLER(RNG_IRQHandler)
DEFAULT_IRQ_HANDLER(FPU_IRQHandler)
DEFAULT_IRQ_HANDLER(HASH_IRQHandler)
DEFAULT_IRQ_HANDLER(PKA_IRQHandler)
DEFAULT_IRQ_HANDLER(SPI3_IRQHandler)
DEFAULT_IRQ_HANDLER(ICACHE_IRQHandler)
DEFAULT_IRQ_HANDLER(ADC4_IRQHandler)
DEFAULT_IRQ_HANDLER(RADIO_IRQHandler)
DEFAULT_IRQ_HANDLER(WKUP_IRQHandler)
DEFAULT_IRQ_HANDLER(HSEM_IRQHandler)
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx)
DEFAULT_IRQ_HANDLER(HSEM_IRQHandler_S)
#endif
#if defined(STM32WBA65xx)
DEFAULT_IRQ_HANDLER(HSEM_S_IRQHandler)
DEFAULT_IRQ_HANDLER(WKUP_S_IRQHandler)
DEFAULT_IRQ_HANDLER(RCC_AUDIOSYNC_IRQHandler)
DEFAULT_IRQ_HANDLER(TIM4_IRQHandler)
DEFAULT_IRQ_HANDLER(I2C2_EV_IRQHandler)
DEFAULT_IRQ_HANDLER(I2C2_ER_IRQHandler)
DEFAULT_IRQ_HANDLER(SPI2_IRQHandler)
DEFAULT_IRQ_HANDLER(OTG_HS_IRQHandler)
DEFAULT_IRQ_HANDLER(I2C4_EV_IRQHandler)
DEFAULT_IRQ_HANDLER(I2C4_ER_IRQHandler)
DEFAULT_IRQ_HANDLER(USART3_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI19_RADIO_IO_IRQHandler)
DEFAULT_IRQ_HANDLER(EXTI20_RADIO_IO_IRQHandler)
#endif
/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

extern const pFunc __VECTOR_TABLE[];
const pFunc __VECTOR_TABLE[] __VECTOR_TABLE_ATTRIBUTE = {
    (pFunc) (&__INITIAL_SP), /*      Initial Stack Pointer */
    Reset_Handler,           /*      Reset Handler */
    NMI_Handler,             /* -14: NMI Handler */
    HardFault_Handler,       /* -13: Hard Fault Handler */
    MemManage_Handler,       /* -12: MPU Fault Handler */
    BusFault_Handler,        /* -11: Bus Fault Handler */
    UsageFault_Handler,      /* -10: Usage Fault Handler */
    SecureFault_Handler,     /*  -9: Secure Fault Handler */
    0,                       /*      Reserved */
    0,                       /*      Reserved */
    0,                       /*      Reserved */
    SVC_Handler,             /*  -5: SVCall Handler */
    DebugMon_Handler,        /*  -4: Debug Monitor Handler */
    0,                       /*      Reserved */
    PendSV_Handler,          /*  -2: PendSV Handler */
    SysTick_Handler,         /*  -1: SysTick Handler */
    WWDG_IRQHandler,         /*   0: Window WatchDog */
    PVD_AVD_IRQHandler,      /*   1: PVD/AVD through EXTI Line detection Interrupt */
    RTC_IRQHandler,          /*   2: RTC non-secure interrupt */
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx)
    RTC_IRQHandler_S, /*   3: RTC secure interrupt */
#else
    0, /*   3: Reserved */
#endif
    TAMP_IRQHandler,   /*   4: Tamper non-secure interrupt  */
    RAMCFG_IRQHandler, /*   5: RAMCFG global */
    FLASH_IRQHandler,  /*   6: FLASH non-secure global interrupt */
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
    FLASH_IRQHandler_S, /*   7: FLASH secure global interrupt */
    GTZC_IRQHandler,    /*   8: Global TrustZone Controller interrupt */
#else
    0, /*   7: Reserved */
    0, /*   8: Reserved */
#endif
    RCC_IRQHandler, /*   9: RCC non-secure global interrupt */
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx)
    RCC_IRQHandler_S, /*  10: RCC secure global interrupt */
#else
    0, /*  10: Reserved */
#endif
    EXTI0_IRQHandler,  /*  11: EXTI Line0 interrupt */
    EXTI1_IRQHandler,  /*  12: EXTI Line1 interrupt */
    EXTI2_IRQHandler,  /*  13: EXTI Line2 interrupt */
    EXTI3_IRQHandler,  /*  14: EXTI Line3 interrupt */
    EXTI4_IRQHandler,  /*  15: EXTI Line4 interrupt */
    EXTI5_IRQHandler,  /*  16: EXTI Line5 interrupt */
    EXTI6_IRQHandler,  /*  17: EXTI Line6 interrupt */
    EXTI7_IRQHandler,  /*  18: EXTI Line7 interrupt */
    EXTI8_IRQHandler,  /*  19: EXTI Line8 interrupt */
    EXTI9_IRQHandler,  /*  20: EXTI Line9 interrupt */
    EXTI10_IRQHandler, /*  21: EXTI Line10 interrupt */
    EXTI11_IRQHandler, /*  22: EXTI Line11 interrupt */
    EXTI12_IRQHandler, /*  23: EXTI Line12 interrupt */
    EXTI13_IRQHandler, /*  24: EXTI Line13 interrupt */
    EXTI14_IRQHandler, /*  25: EXTI Line14 interrupt */
    EXTI15_IRQHandler, /*  26: EXTI Line15 interrupt */
    IWDG_IRQHandler,   /*  27: IWDG global interrupt */
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
    SAES_IRQHandler, /*  28: Secure AES global interrupt */
#else
    0, /*  28: Reserved */
#endif
    GPDMA1_Channel0_IRQHandler, /*  29: GPDMA1 Channel 0 global interrupt */
    GPDMA1_Channel1_IRQHandler, /*  30: GPDMA1 Channel 1 global interrupt */
    GPDMA1_Channel2_IRQHandler, /*  31: GPDMA1 Channel 2 global interrupt */
    GPDMA1_Channel3_IRQHandler, /*  32: GPDMA1 Channel 3 global interrupt */
    GPDMA1_Channel4_IRQHandler, /*  33: GPDMA1 Channel 4 global interrupt */
    GPDMA1_Channel5_IRQHandler, /*  34: GPDMA1 Channel 5 global interrupt */
    GPDMA1_Channel6_IRQHandler, /*  35: GPDMA1 Channel 6 global interrupt */
    GPDMA1_Channel7_IRQHandler, /*  36: GPDMA1 Channel 7 global interrupt */
    TIM1_BRK_IRQHandler,        /*  37: TIM1 Break interrupt */
    TIM1_UP_IRQHandler,         /*  38: TIM1 Update interrupt */
    TIM1_TRG_COM_IRQHandler,    /*  39: TIM1 Trigger and Commutation interrupt */
    TIM1_CC_IRQHandler,         /*  40: TIM1 Capture Compare interrupt */
    TIM2_IRQHandler,            /*  41: TIM2 global interrupt */
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
    TIM3_IRQHandler,    /*  42: TIM3 global interrupt */
    I2C1_EV_IRQHandler, /*  43: I2C1 Event interrupt */
    I2C1_ER_IRQHandler, /*  44: I2C1 Error interrupt */
    SPI1_IRQHandler,    /*  45: SPI1 global interrupt */
#else
    0, /*  42: Reserved */
    0, /*  43: Reserved */
    0, /*  44: Reserved */
    0, /*  45: Reserved */
#endif
    USART1_IRQHandler, /*  46: USART1 global interrupt */
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
    USART2_IRQHandler, /*  47: USART2 global interrupt */
#else
    0, /*  47: Reserved */
#endif
    LPUART1_IRQHandler, /*  48: LPUART1 global interrupt */
    LPTIM1_IRQHandler,  /*  49: LPTIM1 global interrupt */
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
    LPTIM2_IRQHandler, /*  50: LPTIM2 global interrupt */
#else
    0, /*  50: Reserved */
#endif
    TIM16_IRQHandler, /*  51: TIM16 global interrupt */
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
    TIM17_IRQHandler, /*  52: TIM17 global interrupt */
#else
    0, /*  52: Reserved */
#endif
#if defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
    COMP_IRQHandler, /*  53: COMP1 and COMP2 through EXTI Lines interrupt */
#else
    0, /*  53: Reserved */
#endif
    I2C3_EV_IRQHandler, /*  54: I2C3 event */
    I2C3_ER_IRQHandler, /*  55: I2C3 error */
#if defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
    SAI1_IRQHandler, /*  56: Serial Audio Interface 1 global interrupt */
#else
    0, /*  56: Reserved */
#endif
    TSC_IRQHandler,    /*  57: Touch Sense Controller global interrupt */
    AES_IRQHandler,    /*  58: AES global interrupt */
    RNG_IRQHandler,    /*  59: RNG global interrupt */
    FPU_IRQHandler,    /*  60: FPU global interrupt */
    HASH_IRQHandler,   /*  61: HASH global interrupt */
    PKA_IRQHandler,    /*  62: PKA global interrupt */
    SPI3_IRQHandler,   /*  63: SPI3 global interrupt */
    ICACHE_IRQHandler, /*  64: Instruction cache global interrupt */
    ADC4_IRQHandler,   /*  65: LP ADC (12bits) global interrupt */
    RADIO_IRQHandler,  /*  66: 2.4GHz RADIO global interrupt */
    WKUP_IRQHandler,   /*  67: PWR global WKUP pin interrupt */
    HSEM_IRQHandler,   /*  68: HSEM non-secure global interrupt */
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx)
    HSEM_IRQHandler_S, /*  69: HSEM secure global interrupt */
#else
    0, /*  69: Reserved */
#endif
#if defined(STM32WBA65xx)
    HSEM_S_IRQHandler,          /*  70: HSEM secure global interrupt */
    WKUP_S_IRQHandler,          /*  71: PWR secure global WKUP pin interrupt */
    RCC_AUDIOSYNC_IRQHandler,   /*  72: RCC audio synchronization interrupt */
    TIM4_IRQHandler,            /*  73: TIM4 global interrupt */
    I2C2_EV_IRQHandler,         /*  74: I2C2 event interrupt */
    I2C2_ER_IRQHandler,         /*  75: I2c2 error interrupt */
    SPI2_IRQHandler,            /*  76: SPI2 global interrupt */
    OTG_HS_IRQHandler,          /*  77: USB OTG_HS global interrupt */
    I2C4_EV_IRQHandler,         /*  78: I2C4 event global interrupt */
    I2C4_ER_IRQHandler,         /*  79: I2C4 error global interrupt */
    USART3_IRQHandler,          /*  80: USART3 global interrupt */
    EXTI19_RADIO_IO_IRQHandler, /*  81: EXTI line 19 interrupt, 2.4 GHz RADIO io[x] */
    EXTI20_RADIO_IO_IRQHandler, /*  82: EXTI line 20 interrupt, 2.4 GHz RADIO io[y] */
#endif
};

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
void Reset_Handler(void)
{
#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    __IO uint32_t tmp;

#endif
#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    /* disable IRQ is removed */
    /*__disable_irq();*/
    /* Tamp IRQ prio is set to highest , and IRQ is enabled */
    NVIC_SetPriority(TAMP_IRQn, 0);
    NVIC_EnableIRQ(TAMP_IRQn);
#endif
    __set_MSPLIM((uint32_t) (&__STACK_LIMIT));
#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    SCB->VTOR = (uint32_t) &__VECTOR_TABLE[0];
    /* Lock Secure Vector Table */
    /* Enable SYSCFG interface clock */
    RCC->APB7ENR |= RCC_APB7ENR_SYSCFGEN;
    /* Delay after an RCC peripheral clock enabling */
    tmp = RCC->APB7ENR;
    (void) tmp;
    SYSCFG->CSLCKR |= SYSCFG_CSLCKR_LOCKSVTAIRCR;
#endif
    SystemInit();      /* CMSIS System Initialization */
    __PROGRAM_START(); /* Enter PreMain (C library entry point) */
}
