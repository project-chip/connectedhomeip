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
#ifndef SIMPLE_NVM_ARBITER_CONF_H
#define SIMPLE_NVM_ARBITER_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "utilities_common.h"

#include "simple_nvm_arbiter_common.h"

#include "stm32wbaxx_hal_flash.h"

/* Exported constants --------------------------------------------------------*/

/* ========================================================================== */
/* +                            NVM part - USER DEFINED                     + */
/* ========================================================================== */

/**
 * @brief Number of managed NVMs
 *
 * @details This number must be lower than SNVMA_MAX_NUMBER_NVM
 *
 */
#define SNVMA_NVM_NUMBER 1u

/* Check that NVM number does not exceed limitations */
#if SNVMA_NVM_NUMBER > SNVMA_MAX_NUMBER_NVM
#error Number of NVM to manage is to high
#endif /* SNVMA_NVM_NUMBER > SNVMA_MAX_NUMBER_NVM */

/* ========================================================================== */
/* +                        NVM IDs part - USER DEFINED                     + */
/* ========================================================================== */

/* NVM ID #1 */
#define SNVMA_NVM_ID_1
#define SNVMA_NVM_ID_1_BANK_NUMBER 2u
#define SNVMA_NVM_ID_1_BANK_SIZE 1u

#if (SNVMA_NVM_ID_1_BANK_NUMBER == 0u) || (SNVMA_NVM_ID_1_BANK_SIZE == 0u)
#error NVM ID #1 => Bank not initialized
#elif (SNVMA_NVM_ID_1_BANK_NUMBER < SNVMA_MIN_NUMBER_BANK)
#error NVM ID #1 => Not enough bank
#endif

/* ========================================================================== */
/* +                       Check part - NOT USER DEFINED                    + */
/* ========================================================================== */

/* Compute the number of sectors required */

#define SNVMA_NUMBER_OF_SECTOR_NEEDED (SNVMA_NVM_ID_1_BANK_NUMBER * SNVMA_NVM_ID_1_BANK_SIZE)

#define SNVMA_NUMBER_OF_BANKS (SNVMA_NVM_ID_1_BANK_NUMBER)

/* Check that required number of sector does not exceed Flash capacities */
#if SNVMA_NUMBER_OF_SECTOR_NEEDED == 0u
#error SNVMA_NUMBER_OF_SECTOR_NEEDED shall not be zero
#elif SNVMA_NUMBER_OF_BANKS == 0u
#error SNVMA_NUMBER_OF_BANKS shall not be zero
#endif /* SNVMA_NUMBER_OF_SECTOR_NEEDED == 0 */

/* Exported types ------------------------------------------------------------*/

/* ========================================================================== */
/* +                    Buffer ID part - CAN BE USER DEFINED                + */
/* ========================================================================== */

/**
 * @brief Enumeration of the Buffer IDs available
 *
 * @details Each NVM can handle up to 4 user IDs
 *
 * @details Enumeration member can be renamed to fit user needs - ie: SNVMA_BufferId_4 => SNVMA_BleNvmId
 *
 */
typedef enum SNVMA_BufferId
{
    APP_BLE_NvmBuffer,
    SNVMA_BufferId_Max /* End of the enumeration */
} SNVMA_BufferId_t;

/* Exported variables --------------------------------------------------------*/
extern SNVMA_NvmElt_t SNVMA_NvmConfiguration[SNVMA_NVM_NUMBER];

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* SIMPLE_NVM_ARBITER_CONF_H */
