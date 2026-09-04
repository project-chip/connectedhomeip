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
#ifndef STM_EXT_FLASH_H
#define STM_EXT_FLASH_H

/* Includes ------------------------------------------------------------------*/
#include "utilities_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXTERNAL_FLASH_ADDRESS 0x90000000U
#define OTA_MAX_SIZE 0x100000 // 1 Mbytes

typedef enum
{
    STM_EXT_FLASH_OK,
    STM_EXT_FLASH_INIT_FAILED,
    STM_EXT_FLASH_WRITE_FAILED,
    STM_EXT_FLASH_READ_FAILED,
    STM_EXT_FLASH_DELETE_FAILED,
    STM_EXT_FLASH_INVALID_PARAM,
    STM_EXT_FLASH_SIZE_FULL
} STM_OTA_StatusTypeDef;

/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

/**
 * @brief  init ota fw
 */
STM_OTA_StatusTypeDef STM_EXT_FLASH_Init(void);

/**
 * @brief  Delete old image in external flash
 */
STM_OTA_StatusTypeDef STM_EXT_FLASH_Delete_Image(uint32_t Address, uint32_t Length);

/**
 * @brief  Write chunk of data in external flash
 */
STM_OTA_StatusTypeDef STM_EXT_FLASH_WriteChunk(uint32_t DestAddress, uint8_t * pSrcBuffer, uint32_t Length);

/**
 * @brief  Read chunk of data in external flash
 */
STM_OTA_StatusTypeDef STM_EXT_FLASH_ReadChunk(uint32_t DestAddress, uint8_t * pSrcBuffer, uint32_t Length);

#ifdef __cplusplus
}
#endif

#endif /*STM_EXT_FLASH_H */
