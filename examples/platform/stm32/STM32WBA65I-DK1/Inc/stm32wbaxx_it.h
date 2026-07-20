/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32wbaxx_it.h
 * @brief   This file contains the headers of the interrupt handlers.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32WBAxx_IT_H
#define __STM32WBAxx_IT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);
void RTC_IRQHandler(void);
void RCC_IRQHandler(void);
void GPDMA1_Channel0_IRQHandler(void);
void GPDMA1_Channel1_IRQHandler(void);
void GPDMA1_Channel2_IRQHandler(void);
void GPDMA1_Channel3_IRQHandler(void);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void TIM16_IRQHandler(void);
void PKA_IRQHandler(void);
void ADC4_IRQHandler(void);
void RADIO_IRQHandler(void);
void WKUP_IRQHandler(void);
void COMP_IRQHandler(void);
/* USER CODE BEGIN EFP */
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __STM32WBAxx_IT_H */
