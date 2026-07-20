/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm_ext_flash.h
 * @author  MCD Application Team
 * @brief   Header file for stm_ext_flash.c
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
