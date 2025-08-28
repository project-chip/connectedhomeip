/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    crc_ctrl_conf.c
  * @author  MCD Application Team
  * @brief   Source for CRC client controller module configuration file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
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
