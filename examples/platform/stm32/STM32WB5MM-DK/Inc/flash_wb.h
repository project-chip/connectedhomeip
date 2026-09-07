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
#ifndef FLASH_NVM_H
#define FLASH_NVM_H

/* Includes ------------------------------------------------------------------*/
#include "stm32wbxx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    NVM_OK,
    NVM_KEY_NOT_FOUND,
    NVM_WRITE_FAILED,
    NVM_READ_FAILED,
    NVM_DELETE_FAILED,
    NVM_SIZE_FULL,
    NVM_BLOCK_SIZE_OVERFLOW,
    NVM_ERROR_BLOCK_ALIGN,
    NVM_FLASH_CORRUPTION,
    NVM_PARAM_ERROR,
    NVM_BUFFER_TOO_SMALL
} NVM_StatusTypeDef;

typedef enum
{
    SECTOR_DEFAULT = 0,
    SECTOR_NO_SECURE,
    SECTOR_SECURE

} NVM_Sector;

/* Exported functions ------------------------------------------------------- */

/**
 * @brief  Copy Flash to RAM NVM
 */

NVM_StatusTypeDef NM_Init(void);

/**
 * @brief  Copy RAM NVM to Flash
 */
NVM_StatusTypeDef NM_Dump(void);

/**
 * @brief   Get KeyName in RAM NVM and return the value of Key in KeyValue
 *
 * @param  KeyValue:     Address of the buffer changed in this function if the key found
 * @param  KeyName:  	  Name of Key needed
 * @param  KeySize:   	  size of KeyValue
 * @param  read_by_size: return size of KeyValue found
 * @retval return state of function
 */
NVM_StatusTypeDef NM_GetKeyValue(void * KeyValue, const char * KeyName, uint32_t KeySize, size_t * read_by_size, NVM_Sector sector);

/**
 * @brief   Set KeyName and value in RAM NVM
 *
 * @param  KeyValue:     Address of the buffer
 * @param  KeyName:  	  Name of Key needed
 * @param  KeySize:   	  size of KeyValue
 * @param  read_by_size: return size of KeyValue found
 * @retval return state of function
 */

NVM_StatusTypeDef NM_SetKeyValue(char * KeyValue, char * KeyName, uint32_t KeySize, NVM_Sector sector);

/**
 * @brief  Delete Key in RAM NVM
 * @param  KeyName:  	  Name of Key needed
 * @retval return state of function
 */

NVM_StatusTypeDef NM_DeleteKey(const char * Keyname, NVM_Sector sector);

/**
 * @brief  Get the address of the OT NVM buffer
 * @param  Addr: return the Address of the OT buffer
 * @retval return state of function
 */
NVM_StatusTypeDef NM_GetOtNVMAddr(uint32_t * NVMAddr);

/**
 * @brief  Erase all persistent and reboot program
 */
void NM_ResetFactory(void);

#ifdef __cplusplus
}
#endif

#endif /*FLASH_NVM_H */
