/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32WBxx_IT_H
#define __STM32WBxx_IT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void IPCC_C1_RX_IRQHandler(void);
void IPCC_C1_TX_IRQHandler(void);
#if (CFG_HW_USART1_ENABLED == 1)
void USART1_IRQHandler(void);
#endif
#if (CFG_HW_USART1_DMA_TX_SUPPORTED == 1)
void CFG_HW_USART1_DMA_TX_IRQHandler(void);
#endif
#if (CFG_HW_LPUART1_DMA_TX_SUPPORTED == 1)
void CFG_HW_USART1_DMA_TX_IRQHandler(void);
#endif
void RTC_WKUP_IRQHandler(void);
#if (CFG_HW_LPUART1_ENABLED == 1)
void LPUART1_IRQHandler(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STM32WBxx_IT_H */
