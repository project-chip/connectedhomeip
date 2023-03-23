/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "system_si917.h"
#include <stdbool.h>
#include <stdint.h>
//#include "cmsis_gcc.h"

#ifdef BOOTLOADER_ENABLE
#include "api/btl_interface.h"

#endif

#ifdef SL_APP_PROPERTIES
#include "api/application_properties.h"

#endif

#define EXT_IRQ_COUNT 51 /**< Number of External (NVIC) interrupts */ // senthil copied from "efr32mg12p432f1024gl125.h"
#define TOTAL_INTERRUPTS (16 + EXT_IRQ_COUNT)

#ifdef BOOTLOADER_ENABLE
extern MainBootloaderTable_t mainStageTable;

extern void SystemInit2(void);

/*----------------------------------------------------------------------------
 * Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void (*VECTOR_TABLE_Type)(void);
#endif

#ifdef SL_APP_PROPERTIES
extern ApplicationProperties_t sl_app_properties;

/*----------------------------------------------------------------------------
 * Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void (*VECTOR_TABLE_Type)(void);
#endif

/*---------------------------------------------------------------------------
 * External References
 *---------------------------------------------------------------------------*/
extern uint32_t __INITIAL_SP;
extern uint32_t __StackTop;

#ifndef __INITIAL_SP
#define __INITIAL_SP __StackTop
#endif
#ifndef __VECTOR_TABLE_ATTRIBUTE
#define __VECTOR_TABLE_ATTRIBUTE __attribute__((used, section(".vectors")))
#endif

extern void __PROGRAM_START(void);

#if defined(__START) && defined(__GNUC__)
extern int __START(void) __attribute__((noreturn)); /* main entry point */
void Zero_Table();
void Copy_Table();
#endif /* __START */

/*---------------------------------------------------------------------------
 * Internal References
 *---------------------------------------------------------------------------*/
void Reset_Handler(void);
void Default_Handler(void);

#if defined(__GNUC__)
#ifndef __STACK_SIZE
#define __STACK_SIZE 0x00000400
#endif /* __STACK_SIZE */

#ifndef __HEAP_SIZE
#define __HEAP_SIZE 0x00000C00
#endif /* __HEAP_SIZE */
#endif /* __GNUC__ */

/*----------------------------------------------------------------------------
 * Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Cortex-M Processor Exceptions */
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));
#ifndef SL_APP_PROPERTIES
/* Provide a dummy value for the sl_app_properties symbol. */
void sl_app_properties(void); /* Prototype to please MISRA checkers. */
void sl_app_properties(void) __attribute__((weak, alias("Default_Handler")));
#endif

/* Part Specific Interrupts */

void EMU_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FRC_PRI_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void WDOG0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void WDOG1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FRC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void MODEM_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RAC_SEQ_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RAC_RSM_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void BUFC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void LDMA_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void GPIO_EVEN_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIMER0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART0_RX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART0_TX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ACMP0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ADC0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void IDAC0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void GPIO_ODD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIMER1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART1_RX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART1_TX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void LEUART0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PCNT0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CMU_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void MSC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CRYPTO0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void LETIMER0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void AGC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PROTIMER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTCC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SYNTH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CRYOTIMER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RFSENSE_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FPUEH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SMU_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void WTIMER0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void WTIMER1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PCNT1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PCNT2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART2_RX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART2_TX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART3_RX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART3_TX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void VDAC0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CSEN_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void LESENSE_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CRYPTO1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TRNG0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

#if 0 // senthil_ccp

#ifndef __PROGRAM_START

/**
  \brief   Initializes data and bss sections
  \details This default implementations initialized all data and additional bss
           sections relying on .copy.table and .zero.table specified properly
           in the used linker script.

 */
void __cmsis_start(void)
{
  extern void _start(void) __NO_RETURN;

  typedef struct {
    uint32_t const* src;
    uint32_t* dest;
    uint32_t  wlen;
  } __copy_table_t;

  typedef struct {
    uint32_t* dest;
    uint32_t  wlen;
  } __zero_table_t;

  extern const __copy_table_t __copy_table_start__;
  extern const __copy_table_t __copy_table_end__;
  extern const __zero_table_t __zero_table_start__;
  extern const __zero_table_t __zero_table_end__;

  for (__copy_table_t const* pTable = &__copy_table_start__; pTable < &__copy_table_end__; ++pTable) {
    for(uint32_t i=0u; i<pTable->wlen; ++i) {
      pTable->dest[i] = pTable->src[i];
    }
  }

  for (__zero_table_t const* pTable = &__zero_table_start__; pTable < &__zero_table_end__; ++pTable) {
    for(uint32_t i=0u; i<pTable->wlen; ++i) {
      pTable->dest[i] = 0u;
    }
  }

  _start();
}

#define __PROGRAM_START __cmsis_start
#endif

#endif / senthil_ccp
/*----------------------------------------------------------------------------
 * Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#if defined(__ICCARM__)
#pragma data_alignment = 512
extern const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS];
const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS] __VECTOR_TABLE_ATTRIBUTE = {
#elif defined(__GNUC__)
extern const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS];
// const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS] __attribute__((aligned(512))) __VECTOR_TABLE_ATTRIBUTE = {
const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS] __attribute__((aligned(512))) __VECTOR_TABLE_ATTRIBUTE = {
#else
extern const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS];
const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS] __VECTOR_TABLE_ATTRIBUTE = {
#endif
    /* Cortex-M Exception Handlers */
    { .topOfStack = &__INITIAL_SP }, /* Initial Stack Pointer */
    { Reset_Handler },               /* Reset Handler */
    { NMI_Handler },                 /* NMI Handler */
    { HardFault_Handler },           /* Hard Fault Handler */
    { MemManage_Handler },           /* MPU Fault Handler */
    { BusFault_Handler },            /* Bus Fault Handler */
    { UsageFault_Handler },          /* Usage Fault Handler */
    { Default_Handler },             /* Reserved */
    { Default_Handler },             /* Reserved */
    { Default_Handler },             /* Reserved */
#ifdef BOOTLOADER_ENABLE
    { (VECTOR_TABLE_Type) &mainStageTable },
#else
    { Default_Handler },   /* Reserved */
#endif
    { SVC_Handler },      /* SVCall Handler */
    { DebugMon_Handler }, /* Debug Monitor Handler */
#ifdef SL_APP_PROPERTIES
    { (VECTOR_TABLE_Type) &sl_app_properties }, /* Application properties*/
#else
    { sl_app_properties }, /* Application properties*/
#endif
    { PendSV_Handler },  /* PendSV Handler */
    { SysTick_Handler }, /* SysTick Handler */

    /* External interrupts */

    { EMU_IRQHandler },       /* 0 */
    { FRC_PRI_IRQHandler },   /* 1 */
    { WDOG0_IRQHandler },     /* 2 */
    { WDOG1_IRQHandler },     /* 3 */
    { FRC_IRQHandler },       /* 4 */
    { MODEM_IRQHandler },     /* 5 */
    { RAC_SEQ_IRQHandler },   /* 6 */
    { RAC_RSM_IRQHandler },   /* 7 */
    { BUFC_IRQHandler },      /* 8 */
    { LDMA_IRQHandler },      /* 9 */
    { GPIO_EVEN_IRQHandler }, /* 10 */
    { TIMER0_IRQHandler },    /* 11 */
    { USART0_RX_IRQHandler }, /* 12 */
    { USART0_TX_IRQHandler }, /* 13 */
    { ACMP0_IRQHandler },     /* 14 */
    { ADC0_IRQHandler },      /* 15 */
    { IDAC0_IRQHandler },     /* 16 */
    { I2C0_IRQHandler },      /* 17 */
    { GPIO_ODD_IRQHandler },  /* 18 */
    { TIMER1_IRQHandler },    /* 19 */
    { USART1_RX_IRQHandler }, /* 20 */
    { USART1_TX_IRQHandler }, /* 21 */
    { LEUART0_IRQHandler },   /* 22 */
    { PCNT0_IRQHandler },     /* 23 */
    { CMU_IRQHandler },       /* 24 */
    { MSC_IRQHandler },       /* 25 */
    { CRYPTO0_IRQHandler },   /* 26 */
    { LETIMER0_IRQHandler },  /* 27 */
    { AGC_IRQHandler },       /* 28 */
    { PROTIMER_IRQHandler },  /* 29 */
    { RTCC_IRQHandler },      /* 30 */
    { SYNTH_IRQHandler },     /* 31 */
    { CRYOTIMER_IRQHandler }, /* 32 */
    { RFSENSE_IRQHandler },   /* 33 */
    { FPUEH_IRQHandler },     /* 34 */
    { SMU_IRQHandler },       /* 35 */
    { WTIMER0_IRQHandler },   /* 36 */
    { WTIMER1_IRQHandler },   /* 37 */
    { PCNT1_IRQHandler },     /* 38 */
    { PCNT2_IRQHandler },     /* 39 */
    { USART2_RX_IRQHandler }, /* 40 */
    { USART2_TX_IRQHandler }, /* 41 */
    { I2C1_IRQHandler },      /* 42 */
    { USART3_RX_IRQHandler }, /* 43 */
    { USART3_TX_IRQHandler }, /* 44 */
    { VDAC0_IRQHandler },     /* 45 */
    { CSEN_IRQHandler },      /* 46 */
    { LESENSE_IRQHandler },   /* 47 */
    { CRYPTO1_IRQHandler },   /* 48 */
    { TRNG0_IRQHandler },     /* 49 */
    { Default_Handler },      /* 50 - Reserved */
};

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

//#if defined (__START) && defined (__GNUC__) //senthil_ccp
void Copy_Table()
{
    uint32_t *pSrc, *pDest;
    extern uint32_t __etext;
    extern uint32_t __data_start__;
    extern uint32_t __data_end__;
    pSrc  = &__etext;
    pDest = &__data_start__;

    for (; pDest < &__data_end__;)
    {
        *pDest++ = *pSrc++;
    }
}

void Zero_Table()
{
    uint32_t * pDest;
    extern uint32_t __bss_start__;
    extern uint32_t __bss_end__;
    pDest = &__bss_start__;

    for (; pDest < &__bss_end__;)
    {
        *pDest++ = 0UL;
    }
}
//#endif /* __START */

/*---------------------------------------------------------------------------
 * Reset Handler called on controller reset
 *---------------------------------------------------------------------------*/
#if 0
 void SysTick_Handler(void)
{
	SysTick_Handler();
}
#endif

void Reset_Handler(void)
{
#ifndef __NO_SYSTEM_INIT
    SystemInit(); /* CMSIS System Initialization */
#endif

#ifdef BOOTLOADER_ENABLE
    SystemInit2();
#endif /* BOOTLOADER_ENABLE */

#if defined(__GNUC__) && defined(__START)
    Copy_Table();
    Zero_Table();
    __START();
#else
#if 0 // senthil_ccp
  __PROGRAM_START();               /* Enter PreMain (C library entry point) */
#else
    Copy_Table();
    Zero_Table();
    _start();
#endif
#endif /* __GNUC__ */
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif

/*----------------------------------------------------------------------------
 * Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void)
{
    while (true)
    {
    }
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma clang diagnostic pop
#endif
