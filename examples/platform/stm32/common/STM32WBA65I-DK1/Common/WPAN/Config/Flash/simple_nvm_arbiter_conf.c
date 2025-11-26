/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    simple_nvm_arbiter_conf.c
  * @author  MCD Application Team
  * @brief   The Simple NVM arbiter module provides an interface to write and/or
  *          restore data from SRAM to FLASH with use of NVMs.
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

/* Memset */
#include <string.h>

/* Own header files */
#include "simple_nvm_arbiter_conf.h"
#include "simple_nvm_arbiter_common.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Representation of the NVM configuration */
SNVMA_NvmElt_t SNVMA_NvmConfiguration [SNVMA_NVM_NUMBER] =
{
  /* NVM ID #1 */
  {
    .BankNumber = SNVMA_NVM_ID_1_BANK_NUMBER,
    .BankSize = SNVMA_NVM_ID_1_BANK_SIZE,
  },
  /* NVM ID #2 */
  {
    .BankNumber = SNVMA_NVM_ID_2_BANK_NUMBER,
    .BankSize = SNVMA_NVM_ID_2_BANK_SIZE,
  },
};

/* Callback prototypes -------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
/* Callback Definition -------------------------------------------------------*/
/* Private functions Definition ----------------------------------------------*/
