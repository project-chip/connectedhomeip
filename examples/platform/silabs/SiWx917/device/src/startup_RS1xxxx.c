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

#include "sli_siwx917_soc.h"
#include "system_si917.h"

/*----------Stack Configuration-----------------------------------------------*/
#define STACK_SIZE 0x00000C00 /*!< Stack size (in Words)           */
__attribute__((section(".co_stack"))) unsigned long pulStack[STACK_SIZE];

#define EXT_IRQ_COUNT 98 /**< Number of External (NVIC) interrupts */ // senthil copied from "efr32mg12p432f1024gl125.h"
#define TOTAL_INTERRUPTS (16 + EXT_IRQ_COUNT)

#ifndef __INITIAL_SP
#define __INITIAL_SP __StackTop
#endif
#ifndef __VECTOR_TABLE_ATTRIBUTE
#define __VECTOR_TABLE_ATTRIBUTE __attribute__((used, section(".vectors")))
#endif

/*----------Macro definition--------------------------------------------------*/
#define WEAK __attribute__((weak))

/*----------Declaration of the default fault handlers-------------------------*/
#ifndef __START
extern void _start(void) __attribute__((noreturn)); /* Pre Main (C library entry point) */
#else
extern int __START(void) __attribute__((noreturn)); /* main entry point */
#endif

/* System exception vector handler */
__attribute__((used)) void WEAK Reset_Handler(void);
void WEAK NMI_Handler(void);
void WEAK HardFault_Handler(void);
void WEAK MemManage_Handler(void);
void WEAK BusFault_Handler(void);
void WEAK UsageFault_Handler(void);
void WEAK SVC_Handler(void);
void WEAK DebugMon_Handler(void);
void WEAK PendSV_Handler(void);
void WEAK SysTick_Handler(void);

/*Boot up functions*/
void RSI_Default_Reset_Handler(void);
void RSI_Default_WakeUp_Handler(void);
void RSI_PS_RestoreCpuContext(void);

void WEAK IRQ000_Handler(void);                                           /*!<  ULP Processor Interrupt   0      */
void WEAK IRQ001_Handler(void);                                           /*!<  ULP Processor Interrupt   1      */
void WEAK IRQ002_Handler(void);                                           /*!<  ULP Processor Interrupt   2      */
void WEAK IRQ003_Handler(void);                                           /*!<  ULP Processor Interrupt   3      */
void WEAK IRQ004_Handler(void);                                           /*!<  ULP Processor Interrupt   4      */
void WEAK IRQ005_Handler(void);                                           /*!<  ULP Processor Interrupt   5      */
void WEAK IRQ006_Handler(void);                                           /*!<  ULP Processor Interrupt   6      */
void WEAK IRQ007_Handler(void);                                           /*!<  ULP Processor Interrupt   7      */
void WEAK IRQ008_Handler(void);                                           /*!<  ULP Processor Interrupt   8      */
void WEAK IRQ009_Handler(void);                                           /*!<  ULP Processor Interrupt   9      */
void WEAK IRQ010_Handler(void);                                           /*!<  ULP Processor Interrupt   10     */
void WEAK IRQ011_Handler(void);                                           /*!<  ULP Processor Interrupt   11     */
void WEAK IRQ012_Handler(void);                                           /*!<  ULP Processor Interrupt   12     */
void WEAK IRQ013_Handler(void);                                           /*!<  ULP Processor Interrupt   13     */
void WEAK IRQ014_Handler(void);                                           /*!<  ULP Processor Interrupt   14     */
void WEAK IRQ015_Handler(void);                                           /*!<  ULP Processor Interrupt   15     */
void WEAK IRQ016_Handler(void);                                           /*!<  ULP Processor Interrupt   15     */
void WEAK IRQ017_Handler(void);                                           /*!<  ULP Processor Interrupt   17     */
void WEAK IRQ018_Handler(void);                                           /*!<  ULP Processor Interrupt   18     */
void WEAK IRQ019_Handler(void);                                           /*!<  ULP Processor Interrupt   19     */
void WEAK IRQ020_Handler(void); /*!<  Sleep Sensor Interrupts   0      */ /*WDT*/
void WEAK IRQ021_Handler(void);                                           /*!<  Sleep Sensor Interrupts   1      */
void WEAK IRQ022_Handler(void);                                           /*!<  Sleep Sensor Interrupts   2      */
void WEAK IRQ023_Handler(void);                                           /*!<  Sleep Sensor Interrupts   3      */
void WEAK IRQ024_Handler(void);                                           /*!<  Sleep Sensor Interrupts   4      */
void WEAK IRQ025_Handler(void);                                           /*!<  Sleep Sensor Interrupts   5      */
void WEAK IRQ026_Handler(void);                                           /*!<  Sleep Sensor Interrupts   6      */
void WEAK IRQ027_Handler(void);                                           /*!<  Sleep Sensor Interrupts   7      */
void WEAK IRQ028_Handler(void); /*!<  Sleep Sensor Interrupts   8      */ /*Alarm*/
void WEAK IRQ029_Handler(void); /*!<  Sleep Sensor Interrupts   9      */ /*Msec and sec interrupt */
void WEAK IRQ030_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ031_Handler(void);                                           /*!<  M4SS DMA interrupt               */
void WEAK IRQ032_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ033_Handler(void);                                           /*!<  M4SS DMA interrupt               */
void WEAK IRQ034_Handler(void);                                           /*!<  M4SS SCT interrupt               */
void WEAK HIF1_IRQHandler(void);                                          /*!<  HIF Interrupt 1                  */
void WEAK HIF2_IRQHandler(void);                                          /*!<  HIF Interrupt 2                  */
void WEAK IRQ037_Handler(void);                                           /*!<  SIO Interrupt                    */
void WEAK IRQ038_Handler(void);                                           /*!<  USART 1 Interrupt                */
void WEAK IRQ039_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ040_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ041_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ042_Handler(void);                                           /*!<  I2C Interrupt                    */
void WEAK IRQ043_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ044_Handler(void);                                           /*!<  SSI Slave Interrupt              */
void WEAK IRQ045_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ046_Handler(void);                                           /*!<  GSPI Master 1 Interrupt          */
void WEAK IRQ047_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ048_Handler(void);                                           /*!<  MCPWM Interrupt                  */
void WEAK IRQ049_Handler(void);                                           /*!<  QEI Interrupt                    */
void WEAK IRQ050_Handler(void);                                           /*!<  GPIO Group Interrupt 0           */
void WEAK IRQ051_Handler(void);                                           /*!<  GPIO Group Interrupt 1           */
void WEAK IRQ052_Handler(void);                                           /*!<  GPIO Pin Interrupt   0           */
void WEAK IRQ053_Handler(void);                                           /*!<  GPIO Pin Interrupt   1           */
void WEAK IRQ054_Handler(void);                                           /*!<  GPIO Pin Interrupt   2           */
void WEAK IRQ055_Handler(void);                                           /*!<  GPIO Pin Interrupt   3           */
void WEAK IRQ056_Handler(void);                                           /*!<  GPIO Pin Interrupt   4           */
void WEAK IRQ057_Handler(void);                                           /*!<  GPIO Pin Interrupt   5           */
void WEAK IRQ058_Handler(void);                                           /*!<  GPIO Pin Interrupt   6           */
void WEAK IRQ059_Handler(void);                                           /*!<  GPIO Pin Interrupt   7           */
void WEAK IRQ060_Handler(void);                                           /*!<  QSPI Interrupt                   */
void WEAK IRQ061_Handler(void);                                           /*!<  I2C 2 Interrupt                  */
void WEAK IRQ062_Handler(void);                                           /*!<  Ethernet Interrupt               */
void WEAK IRQ063_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ064_Handler(void);                                           /*!<  I2S master Interrupt             */
void WEAK IRQ065_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ066_Handler(void);                                           /*!<  Can 1 Interrupt                  */
void WEAK IRQ067_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ068_Handler(void);                                           /*!<  SDMEM Interrupt                  */
void WEAK IRQ069_Handler(void);                                           /*!<  PLL clock ind Interrupt          */
void WEAK IRQ070_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ071_Handler(void);                                           /*!<  CCI system Interrupt Out         */
void WEAK IRQ072_Handler(void);                                           /*!<  FPU exception                    */
void WEAK IRQ073_Handler(void);                                           /*!<  USB INTR                         */
void WEAK IRQ074_Handler(void);                                           /*!<  TASS_P2P_INTR                    */
void WEAK IRQ075_Handler(void);                                           /*!<  WLAN Band1 intr0                 */
void WEAK IRQ076_Handler(void);                                           /*!<  WLAN Band1 intr1                 */
void WEAK IRQ077_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ078_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ079_Handler(void);                                           /*!<  BT intr                          */
void WEAK IRQ080_Handler(void);                                           /*!<  ZB intr                          */
void WEAK IRQ081_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ082_Handler(void);                                           /*!<  Modem disabled mode trigger intr */
void WEAK IRQ083_Handler(void);                                           /*!<  gpio intr                        */
void WEAK IRQ084_Handler(void);                                           /*!<  uart intr                        */
void WEAK IRQ085_Handler(void);                                           /*!<  watch dog level intr             */
void WEAK IRQ086_Handler(void);                                           /*!<  ULP Sleep sensor interrupt       */
void WEAK IRQ087_Handler(void);                                           /*!<  ECDH intr                        */
void WEAK IRQ088_Handler(void);                                           /*!<  DH intr                          */
void WEAK IRQ089_Handler(void);                                           /*!<  QSPI intr                        */
void WEAK IRQ090_Handler(void);                                           /*!<  ULP processor interrupt TASS     */
void WEAK IRQ091_Handler(void);                                           /*!<  Sys Tick Timer                   */
void WEAK IRQ092_Handler(void);                                           /*!<  Real Timer interrupt             */
void WEAK IRQ093_Handler(void);                                           /*!<  PLL lock interrupt               */
void WEAK IRQ094_Handler(void);                                           /*!<  Reserved                         */
void WEAK IRQ095_Handler(void);                                           /*!<  UART2 Interrupt                  */
void WEAK IRQ096_Handler(void);                                           /*!<  I2S Interrupt                    */
void WEAK IRQ097_Handler(void);                                           /*!<  I2C Interrupt                    */
void WEAK IRQ098_Handler(void);                                           /*!<  RESERVED                         */

/*----------Symbols defined in linker script----------------------------------*/
extern unsigned long _sidata; /*!< Start address for the initialization
                                      values of the .data section.            */
extern unsigned long _sdata;  /*!< Start address for the .data section     */
extern unsigned long _edata;  /*!< End address for the .data section       */
extern unsigned long _sbss;   /*!< Start address for the .bss section      */
extern unsigned long _ebss;   /*!< End address for the .bss section        */
extern void _eram;            /*!< End address for ram                     */
extern uint32_t __etext;
extern unsigned long __StackTop;

/*----------Function prototypes-----------------------------------------------*/
extern int main(void);             /*!< The entry point for the application  */
void Default_Reset_Handler(void);  /*!< Default reset handler                */
static void Default_Handler(void); /*!< Default exception handler            */
/**
 *@brief The minimal vector table for a Cortex M4.  Note that the proper constructs
 *       must be placed on this to ensure that it ends up at physical address
 *       0x00000000.
 */

//(void *)0x300001
/*CCP new */
extern const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS];
const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS] __attribute__((aligned(512))) __VECTOR_TABLE_ATTRIBUTE = {
    /* Cortex-M Exception Handlers */
    { .topOfStack = &__INITIAL_SP }, /* Initial Stack Pointer */
    /*{ (void *)&pulStack[STACK_SIZE - 1] },*/
    { (void *) 0x300001 },  /* Reset Handler : IN CCP Platform system start
                                           from bootloader which inturn call reset handler */
    { NMI_Handler },        /* NMI Handler */
    { HardFault_Handler },  /* Hard Fault Handler */
    { MemManage_Handler },  /* MPU Fault Handler */
    { BusFault_Handler },   /* Bus Fault Handler */
    { UsageFault_Handler }, /* Usage Fault Handler */
    { 0 },                  /* Reserved */
    { 0 },                  /* Reserved */
    { 0 },                  /* Reserved */
    { 0 },                  /* Reserved */
    { SVC_Handler },        /* SVCall Handler */
    { DebugMon_Handler },   /* Debug Monitor Handler */
    { 0 },                  /* Application properties*/
    { PendSV_Handler },     /* PendSV Handler */
    { SysTick_Handler },    /* SysTick Handler */

    /* External interrupts */

    { IRQ000_Handler },              /* 0 */
    { IRQ001_Handler },              /* 1 */
    { IRQ002_Handler },              /* 2 */
    { IRQ003_Handler },              /* 3 */
    { IRQ004_Handler },              /* 4 */
    { IRQ005_Handler },              /* 5 */
    { IRQ006_Handler },              /* 6 */
    { IRQ007_Handler },              /* 7 */
    { IRQ008_Handler },              /* 8 */
    { IRQ009_Handler },              /* 9 */
    { IRQ010_Handler },              /* 10 */
    { IRQ011_Handler },              /* 11 */
    { IRQ012_Handler },              /* 12 */
    { IRQ013_Handler },              /* 13 */
    { IRQ014_Handler },              /* 14 */
    { IRQ015_Handler },              /* 15 */
    { IRQ016_Handler },              /* 16 */
    { IRQ017_Handler },              /* 17 */
    { IRQ018_Handler },              /* 18 */
    { IRQ019_Handler },              /* 19 */
    { IRQ020_Handler },              /* 20 */
    { IRQ021_Handler },              /* 21 */
    { IRQ022_Handler },              /* 22 */
    { IRQ023_Handler },              /* 23 */
    { IRQ024_Handler },              /* 24 */
    { IRQ025_Handler },              /* 25 */
    { IRQ026_Handler },              /* 26 */
    { IRQ027_Handler },              /* 27 */
    { IRQ028_Handler },              /* 28 */
    { IRQ029_Handler },              /* 29 */
    { (void *) &__StackTop - 0x0c }, /* 30 */
    { IRQ031_Handler },              /* 31 */
    { RSI_Default_Reset_Handler },   /* 32 */
    { IRQ033_Handler },              /* 33 */
    { IRQ034_Handler },              /* 34 */
    { HIF1_IRQHandler },             /* 35 */
    { HIF2_IRQHandler },             /* 36 */
    { IRQ037_Handler },              /* 37 */
    { IRQ038_Handler },              /* 38 */
    { IRQ039_Handler },              /* 39 */
    { RSI_PS_RestoreCpuContext },    /* 40 */
    { IRQ041_Handler },              /* 41 */
    { IRQ042_Handler },              /* 42 */
    { (void *) 0x10AD10AD },         /* 43 */
    { IRQ044_Handler },              /* 44 */
    { 0 },                           /* 45 */
    { IRQ046_Handler },              /* 46 */
    { IRQ047_Handler },              /* 47 */
    { IRQ048_Handler },              /* 48 */
    { IRQ049_Handler },              /* 49 */
    { IRQ050_Handler },              /* 50 - Reserved */
    { IRQ051_Handler },
    { IRQ052_Handler },
    { IRQ053_Handler },
    { IRQ054_Handler },
    { IRQ055_Handler },
    { IRQ056_Handler },
    { IRQ057_Handler },
    { IRQ058_Handler },
    { IRQ059_Handler },
    { IRQ060_Handler },
    { IRQ061_Handler },
    { IRQ062_Handler },
    { IRQ063_Handler },
    { IRQ064_Handler },
    { 0 },
    { IRQ066_Handler },
    { 0 },
    { IRQ068_Handler },
    { IRQ069_Handler },
    { 0 },
    { IRQ071_Handler },
    { IRQ072_Handler },
    { IRQ073_Handler },
    { IRQ074_Handler },
    { IRQ075_Handler },
    { IRQ076_Handler },
    { 0 },
    { 0 },
    { IRQ079_Handler },
    { IRQ080_Handler },
    { 0 },
    { IRQ082_Handler },
    { IRQ083_Handler },
    { IRQ084_Handler },
    { IRQ085_Handler },
    { IRQ086_Handler },
    { IRQ087_Handler },
    { IRQ088_Handler },
    { IRQ089_Handler },
    { IRQ090_Handler },
    { IRQ091_Handler },
    { IRQ092_Handler },
    { IRQ093_Handler },
    { 0 },
    { IRQ095_Handler },
    { 0 },
    { IRQ097_Handler },
};

/**
 * @brief  This is the code that gets never called, Dummy handler
 * @param  None
 * @retval None
 */
void Default_Reset_Handler(void)
{
    /*Generic Default reset handler for CM4 */
    while (1)
        ;
}

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

void RSI_Default_Reset_Handler(void)
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

/**
 *@brief Provide weak aliases for each Exception handler to the Default_Handler.
 *       As they are weak aliases, any function with the same name will override
 *       this definition.
 */
//#pragma weak Reset_Handler      = RSI_Default_Reset_Handler
#pragma weak NMI_Handler        = Default_Handler
#pragma weak HardFault_Handler  = Default_Handler
#pragma weak MemManage_Handler  = Default_Handler
#pragma weak BusFault_Handler   = Default_Handler
#pragma weak UsageFault_Handler = Default_Handler
#pragma weak SVC_Handler        = Default_Handler
#pragma weak DebugMon_Handler   = Default_Handler
#pragma weak PendSV_Handler     = Default_Handler
#pragma weak SysTick_Handler    = Default_Handler
/*----------------------------------external interrupts------------------------------ */
#pragma weak IRQ000_Handler  = Default_Handler
#pragma weak IRQ001_Handler  = Default_Handler
#pragma weak IRQ002_Handler  = Default_Handler
#pragma weak IRQ003_Handler  = Default_Handler
#pragma weak IRQ004_Handler  = Default_Handler
#pragma weak IRQ005_Handler  = Default_Handler
#pragma weak IRQ006_Handler  = Default_Handler
#pragma weak IRQ007_Handler  = Default_Handler
#pragma weak IRQ008_Handler  = Default_Handler
#pragma weak IRQ009_Handler  = Default_Handler
#pragma weak IRQ010_Handler  = Default_Handler
#pragma weak IRQ011_Handler  = Default_Handler
#pragma weak IRQ012_Handler  = Default_Handler
#pragma weak IRQ013_Handler  = Default_Handler
#pragma weak IRQ014_Handler  = Default_Handler
#pragma weak IRQ015_Handler  = Default_Handler
#pragma weak IRQ016_Handler  = Default_Handler
#pragma weak IRQ017_Handler  = Default_Handler
#pragma weak IRQ018_Handler  = Default_Handler
#pragma weak IRQ019_Handler  = Default_Handler
#pragma weak IRQ020_Handler  = Default_Handler
#pragma weak IRQ021_Handler  = Default_Handler
#pragma weak IRQ022_Handler  = Default_Handler
#pragma weak IRQ023_Handler  = Default_Handler
#pragma weak IRQ024_Handler  = Default_Handler
#pragma weak IRQ025_Handler  = Default_Handler
#pragma weak IRQ026_Handler  = Default_Handler
#pragma weak IRQ027_Handler  = Default_Handler
#pragma weak IRQ028_Handler  = Default_Handler
#pragma weak IRQ029_Handler  = Default_Handler
#pragma weak IRQ030_Handler  = Default_Handler
#pragma weak IRQ031_Handler  = Default_Handler
#pragma weak IRQ032_Handler  = Default_Handler
#pragma weak IRQ033_Handler  = Default_Handler
#pragma weak IRQ034_Handler  = Default_Handler
#pragma weak HIF1_IRQHandler = Default_Handler
#pragma weak HIF2_IRQHandler = Default_Handler
#pragma weak IRQ037_Handler  = Default_Handler
#pragma weak IRQ038_Handler  = Default_Handler
#pragma weak IRQ039_Handler  = Default_Handler
#pragma weak IRQ040_Handler  = Default_Handler
#pragma weak IRQ041_Handler  = Default_Handler
#pragma weak IRQ042_Handler  = Default_Handler
#pragma weak IRQ043_Handler  = Default_Handler
#pragma weak IRQ044_Handler  = Default_Handler
#pragma weak IRQ045_Handler  = Default_Handler
#pragma weak IRQ046_Handler  = Default_Handler
#pragma weak IRQ047_Handler  = Default_Handler
#pragma weak IRQ048_Handler  = Default_Handler
#pragma weak IRQ049_Handler  = Default_Handler
#pragma weak IRQ050_Handler  = Default_Handler
#pragma weak IRQ051_Handler  = Default_Handler
#pragma weak IRQ052_Handler  = Default_Handler
#pragma weak IRQ053_Handler  = Default_Handler
#pragma weak IRQ054_Handler  = Default_Handler
#pragma weak IRQ055_Handler  = Default_Handler
#pragma weak IRQ056_Handler  = Default_Handler
#pragma weak IRQ057_Handler  = Default_Handler
#pragma weak IRQ058_Handler  = Default_Handler
#pragma weak IRQ059_Handler  = Default_Handler
#pragma weak IRQ060_Handler  = Default_Handler
#pragma weak IRQ061_Handler  = Default_Handler
#pragma weak IRQ062_Handler  = Default_Handler
#pragma weak IRQ063_Handler  = Default_Handler
#pragma weak IRQ064_Handler  = Default_Handler
#pragma weak IRQ065_Handler  = Default_Handler
#pragma weak IRQ066_Handler  = Default_Handler
#pragma weak IRQ067_Handler  = Default_Handler
#pragma weak IRQ068_Handler  = Default_Handler
#pragma weak IRQ069_Handler  = Default_Handler
#pragma weak IRQ070_Handler  = Default_Handler
#pragma weak IRQ071_Handler  = Default_Handler
#pragma weak IRQ072_Handler  = Default_Handler
#pragma weak IRQ073_Handler  = Default_Handler
#pragma weak IRQ074_Handler  = Default_Handler
#pragma weak IRQ075_Handler  = Default_Handler
#pragma weak IRQ076_Handler  = Default_Handler
#pragma weak IRQ077_Handler  = Default_Handler
#pragma weak IRQ078_Handler  = Default_Handler
#pragma weak IRQ079_Handler  = Default_Handler
#pragma weak IRQ080_Handler  = Default_Handler
#pragma weak IRQ081_Handler  = Default_Handler
#pragma weak IRQ082_Handler  = Default_Handler
#pragma weak IRQ083_Handler  = Default_Handler
#pragma weak IRQ084_Handler  = Default_Handler
#pragma weak IRQ085_Handler  = Default_Handler
#pragma weak IRQ086_Handler  = Default_Handler
#pragma weak IRQ087_Handler  = Default_Handler
#pragma weak IRQ088_Handler  = Default_Handler
#pragma weak IRQ089_Handler  = Default_Handler
#pragma weak IRQ090_Handler  = Default_Handler
#pragma weak IRQ091_Handler  = Default_Handler
#pragma weak IRQ092_Handler  = Default_Handler
#pragma weak IRQ093_Handler  = Default_Handler
#pragma weak IRQ094_Handler  = Default_Handler
#pragma weak IRQ095_Handler  = Default_Handler
#pragma weak IRQ096_Handler  = Default_Handler
#pragma weak IRQ097_Handler  = Default_Handler
#pragma weak IRQ098_Handler  = Default_Handler

/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop,
 *         preserving the system state for examination by a debugger.
 * @param  None
 * @retval None
 */
static void Default_Handler(void)
{
    /* Go into an infinite loop. */
    while (1)
    {
    }
}

/*********************** (C) COPYRIGHT 2009 Coocox ************END OF FILE*****/
