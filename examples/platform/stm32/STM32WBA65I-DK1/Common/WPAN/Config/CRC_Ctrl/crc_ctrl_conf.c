/* USER CODE BEGIN Header */
/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License,
 * Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy
 * of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to
 * in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
/* Own header files */
#include "crc_ctrl.h"

/* HAL CRC header */
#include "stm32wbaxx_hal_crc.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/**
 * @brief CRC Handle configuration for SNVMA use
 */
CRCCTRL_Handle_t SNVMA_Handle =
{
  .Uid = 0x00,
  .PreviousComputedValue = 0x00,
  .State = HANDLE_NOT_REG,
  .Configuration =
  {
    .DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE,
    .DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE,
    .GeneratingPolynomial = 7607,
    .CRCLength = CRC_POLYLENGTH_16B,
    .InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE,
    .OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE,
    .InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS,
  },
};

/* USER CODE BEGIN User CRC configurations */

/* USER CODE END User CRC configurations */

/* Callback prototypes -------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
/* Callback Definition -------------------------------------------------------*/
/* Private functions Definition ----------------------------------------------*/
