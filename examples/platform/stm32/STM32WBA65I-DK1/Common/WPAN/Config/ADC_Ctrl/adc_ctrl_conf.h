/* USER CODE BEGIN Header */
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
 * @brief Flag to determine whether the dynamic VRefInt is performed or not
 * @details Enable: 1u, Disable: 0u
 */
#define ADCCTRL_USE_DYNAMIC_VREF (0u)

/**
 * @brief Set ADC Clock source
 */
#define ADCTCTRL_SET_CLOCK_SOURCE() LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_HCLK)

/**
 * @brief Peripheral clock enable for ADC
 */
#define ADCCTRL_ENABLE_CLOCK() LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_ADC4)

/**
 * @brief Peripheral clock disable for ADC
 */
#define ADCCTRL_DISABLE_CLOCK() LL_AHB4_GRP1_DisableClock(LL_AHB4_GRP1_PERIPH_ADC4)

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

/* USER CODE BEGIN ADC_CTRL_CONF_H */

/* USER CODE END ADC_CTRL_CONF_H */

#endif /* ADC_CTRL_CONF_H */
