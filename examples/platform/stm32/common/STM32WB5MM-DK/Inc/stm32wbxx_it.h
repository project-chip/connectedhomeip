/**
 ******************************************************************************
 * @file    stm32wbxx_it.h
 * @author  MCD Application Team
 * @brief   This file contains the headers of the interrupt handlers.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2019-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
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
void PendSV_Handler(void);
void SysTick_Handler(void);
void IPCC_C1_RX_IRQHandler(void);
void IPCC_C1_TX_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void USART1_IRQHandler(void);
void DMA2_Channel4_IRQHandler(void);
void CFG_HW_USART1_DMA_TX_IRQHandler(void);
void RTC_WKUP_IRQHandler(void);
void LPUART1_IRQHandler(void);
void DMA1_Channel4_IRQHandler(void);
void QUADSPI_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32WBxx_IT_H */
