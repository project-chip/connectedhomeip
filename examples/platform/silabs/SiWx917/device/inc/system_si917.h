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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "si91x_device.h"
#include "core_cm4.h" /* Cortex-M4 processor and core peripherals */
#include <stdint.h>
#include "cmsis_gcc.h"

/*******************************************************************************
 * @addtogroup Parts
 * @{
 ******************************************************************************/
/*******************************************************************************
 * @addtogroup EFR32 EFR32
 * @{
 ******************************************************************************/

/*******************************************************************************
 ******************************   TYPEDEFS   ***********************************
 ******************************************************************************/

/* Interrupt vectortable entry */
typedef union
{
    void (*VECTOR_TABLE_Type)(void);
    void * topOfStack;
} tVectorEntry;

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/
//#ifndef CCP_SI917_BRINGUP
extern uint32_t SystemCoreClock; /**< System Clock Frequency (Core Clock) */
//#endif /* CCP_SI917_BRINGUP */
extern uint32_t SystemHfrcoFreq; /**< System HFRCO frequency */

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

void Reset_Handler(void);      /**< Reset Handler */
void NMI_Handler(void);        /**< NMI Handler */
void HardFault_Handler(void);  /**< Hard Fault Handler */
void MemManage_Handler(void);  /**< MPU Fault Handler */
void BusFault_Handler(void);   /**< Bus Fault Handler */
void UsageFault_Handler(void); /**< Usage Fault Handler */
void SVC_Handler(void);        /**< SVCall Handler */
void DebugMon_Handler(void);   /**< Debug Monitor Handler */
void PendSV_Handler(void);     /**< PendSV Handler */
void SysTick_Handler(void);    /**< SysTick Handler */

void EMU_IRQHandler(void);       /**< EMU IRQ Handler */
void FRC_PRI_IRQHandler(void);   /**< FRC_PRI IRQ Handler */
void WDOG0_IRQHandler(void);     /**< WDOG0 IRQ Handler */
void WDOG1_IRQHandler(void);     /**< WDOG1 IRQ Handler */
void FRC_IRQHandler(void);       /**< FRC IRQ Handler */
void MODEM_IRQHandler(void);     /**< MODEM IRQ Handler */
void RAC_SEQ_IRQHandler(void);   /**< RAC_SEQ IRQ Handler */
void RAC_RSM_IRQHandler(void);   /**< RAC_RSM IRQ Handler */
void BUFC_IRQHandler(void);      /**< BUFC IRQ Handler */
void LDMA_IRQHandler(void);      /**< LDMA IRQ Handler */
void GPIO_EVEN_IRQHandler(void); /**< GPIO_EVEN IRQ Handler */
void TIMER0_IRQHandler(void);    /**< TIMER0 IRQ Handler */
void USART0_RX_IRQHandler(void); /**< USART0_RX IRQ Handler */
void USART0_TX_IRQHandler(void); /**< USART0_TX IRQ Handler */
void ACMP0_IRQHandler(void);     /**< ACMP0 IRQ Handler */
void ADC0_IRQHandler(void);      /**< ADC0 IRQ Handler */
void IDAC0_IRQHandler(void);     /**< IDAC0 IRQ Handler */
void I2C0_IRQHandler(void);      /**< I2C0 IRQ Handler */
void GPIO_ODD_IRQHandler(void);  /**< GPIO_ODD IRQ Handler */
void TIMER1_IRQHandler(void);    /**< TIMER1 IRQ Handler */
void USART1_RX_IRQHandler(void); /**< USART1_RX IRQ Handler */
void USART1_TX_IRQHandler(void); /**< USART1_TX IRQ Handler */
void LEUART0_IRQHandler(void);   /**< LEUART0 IRQ Handler */
void PCNT0_IRQHandler(void);     /**< PCNT0 IRQ Handler */
void CMU_IRQHandler(void);       /**< CMU IRQ Handler */
void MSC_IRQHandler(void);       /**< MSC IRQ Handler */
void CRYPTO0_IRQHandler(void);   /**< CRYPTO IRQ Handler */
void LETIMER0_IRQHandler(void);  /**< LETIMER0 IRQ Handler */
void AGC_IRQHandler(void);       /**< AGC IRQ Handler */
void PROTIMER_IRQHandler(void);  /**< PROTIMER IRQ Handler */
void RTCC_IRQHandler(void);      /**< RTCC IRQ Handler */
void SYNTH_IRQHandler(void);     /**< SYNTH IRQ Handler */
void CRYOTIMER_IRQHandler(void); /**< CRYOTIMER IRQ Handler */
void RFSENSE_IRQHandler(void);   /**< RFSENSE IRQ Handler */
void FPUEH_IRQHandler(void);     /**< FPUEH IRQ Handler */
void SMU_IRQHandler(void);       /**< SMU IRQ Handler */
void WTIMER0_IRQHandler(void);   /**< WTIMER0 IRQ Handler */
void WTIMER1_IRQHandler(void);   /**< WTIMER1 IRQ Handler */
void PCNT1_IRQHandler(void);     /**< PCNT1 IRQ Handler */
void PCNT2_IRQHandler(void);     /**< PCNT2 IRQ Handler */
void USART2_RX_IRQHandler(void); /**< USART2_RX IRQ Handler */
void USART2_TX_IRQHandler(void); /**< USART2_TX IRQ Handler */
void I2C1_IRQHandler(void);      /**< I2C1 IRQ Handler */
void USART3_RX_IRQHandler(void); /**< USART3_RX IRQ Handler */
void USART3_TX_IRQHandler(void); /**< USART3_TX IRQ Handler */
void VDAC0_IRQHandler(void);     /**< VDAC0 IRQ Handler */
void CSEN_IRQHandler(void);      /**< CSEN IRQ Handler */
void LESENSE_IRQHandler(void);   /**< LESENSE IRQ Handler */
void CRYPTO1_IRQHandler(void);   /**< CRYPTO1 IRQ Handler */
void TRNG0_IRQHandler(void);     /**< TRNG0 IRQ Handler */
void SYSCFG_IRQHandler(void);    /**< SYSCFG IRQ Handler */

uint32_t SystemCoreClockGet(void);

/*******************************************************************************
 * @brief
 *   Update CMSIS SystemCoreClock variable.
 *
 * @details
 *   CMSIS defines a global variable SystemCoreClock
 *that shall hold the core frequency in Hz. If the
 *core frequency is dynamically changed, the variable
 *must be kept updated in order to be CMSIS compliant.
 *
 *   Notice that only if changing the core clock
 *frequency through the EFR CMU API, this variable
 *will be kept updated. This function is only provided
 *   for CMSIS compliance and if a user modifies the
 *the core clock outside the CMU API.
 ******************************************************************************/
#ifndef CCP_SI917_BRINGUP
static __INLINE void SystemCoreClockUpdate(void)
{
    (void) SystemCoreClockGet();
}
#endif /* CCP_SI917_BRINGUP */

uint32_t SystemMaxCoreClockGet(void);

void SystemInit(void);
uint32_t SystemHFClockGet(void);

uint32_t SystemHFXOClockGet(void);
void SystemHFXOClockSet(uint32_t freq);

uint32_t SystemLFRCOClockGet(void);
uint32_t SystemULFRCOClockGet(void);

uint32_t SystemLFXOClockGet(void);
void SystemLFXOClockSet(uint32_t freq);

/** @} End of group */
/** @} End of group Parts */

#ifdef __cplusplus
}
#endif
