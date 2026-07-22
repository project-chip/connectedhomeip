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

/* Includes ------------------------------------------------------------------*/

/* Memset */
#include <string.h>

/* Own header files */
#include "simple_nvm_arbiter_common.h"
#include "simple_nvm_arbiter_conf.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Representation of the NVM configuration */
SNVMA_NvmElt_t SNVMA_NvmConfiguration[SNVMA_NVM_NUMBER] = {
    /* NVM ID #1 */
    {
        .BankNumber = SNVMA_NVM_ID_1_BANK_NUMBER,
        .BankSize   = SNVMA_NVM_ID_1_BANK_SIZE,
    },
};

/* Callback prototypes -------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
/* Callback Definition -------------------------------------------------------*/
/* Private functions Definition ----------------------------------------------*/
