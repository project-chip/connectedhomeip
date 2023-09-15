/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    flash_wb.h
 * @author  MCD Application Team
 * @brief   Header file for flash_wb.c
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

void NM_Init(void);

/**
 * @brief  Copy RAM NVM to Flash
 */
NVM_StatusTypeDef NM_Dump(void);

/**
 * @brief  check the nvm if it s corrupted or not
 * @retval return NVM_OK if nvm is empty or NVM_FLASH_CORRUPTION if it s not empty
 */
NVM_StatusTypeDef NM_Check_Validity(void);

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
 * @param  KeyAddr:   	  TODO DELETED this param
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
 * @brief  Erase all persistent and reboot program
 */

void NM_ResetFactory(void);
void NM_FullErase(void);

#ifdef __cplusplus
}
#endif

#endif /*FLASH_NVM_H */
