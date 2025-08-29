/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    adc_ctrl_conf.h
 * @author  MCD Application Team
 * @brief   Configuration Header for adc_ctrl.c module
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
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
#ifndef ADC_CTRL_CONF_H
#define ADC_CTRL_CONF_H

/* Includes ------------------------------------------------------------------*/
/* Own header files */
#include "adc_ctrl.h"

/* ADC configuration types */
#include "stm32wbaxx_ll_adc.h"

/* Exported defines ----------------------------------------------------------*/
/**
 * @brief Physical address of the ADC to use
 */
#define ADCCTRL_HWADDR ADC4

/**
 * @brief Set ADC Clock source
 */
#define ADCTCTRL_SET_CLOCK_SOURCE() \
    LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_HCLK)

/**
 * @brief Peripheral clock enable for ADC
 */
#define ADCCTRL_ENABLE_CLOCK() \
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_ADC4)

/**
 * @brief Peripheral clock disable for ADC
 */
#define ADCCTRL_DISABLE_CLOCK() \
    LL_AHB4_GRP1_DisableClock(LL_AHB4_GRP1_PERIPH_ADC4)

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
/**
 * @brief Handle used by the temperature request for the LL
 */
extern ADCCTRL_Handle_t LLTempRequest_Handle;
#endif /* (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1) */

/* Exported macros -----------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

#endif /* ADC_CTRL_CONF_H */
