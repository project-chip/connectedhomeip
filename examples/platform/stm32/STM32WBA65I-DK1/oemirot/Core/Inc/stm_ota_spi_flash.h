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
/**
 ******************************************************************************
 * @file    stm_ota_spi_flash.h
 * @author  MCD Application Team
 * @brief   Header file for stm_ota_spi_flash.c
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM_OTA_SPI_FLASH_H
#define STM_OTA_SPI_FLASH_H

/* Includes ------------------------------------------------------------------*/
#include "stm_ota_common.h"
#include "utilities_common.h"

#include "app_conf.h"
#include "appli_region_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (OTA_EXTERNAL_FLASH_ENABLE == 1)

#define NS_IMAGE_SECONDARY_PARTITION_OFFSET (FLASH_AREA_2_OFFSET + FLASH_S_PARTITION_SIZE)
#define S_IMAGE_SECONDARY_PARTITION_OFFSET (FLASH_AREA_2_OFFSET)

/* SLOT A - Non Secure application slot */
#define SLOT_DWL_A_START (SPI_FLASH_BASE_ADDRESS + NS_IMAGE_SECONDARY_PARTITION_OFFSET)
#define SLOT_DWL_A_SIZE FLASH_NS_PARTITION_SIZE
#define SLOT_DWL_A_END (SLOT_DWL_A_START + SLOT_DWL_A_SIZE - 1U)

/* SLOT B - Secure application slot */
#define SLOT_DWL_B_START (SPI_FLASH_BASE_ADDRESS + S_IMAGE_SECONDARY_PARTITION_OFFSET)
#define SLOT_DWL_B_SIZE FLASH_S_PARTITION_SIZE
#define SLOT_DWL_B_END (SLOT_DWL_B_START + SLOT_DWL_B_SIZE - 1U)

/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

/**
 * @brief  init ota fw
 */
STM_OTA_StatusTypeDef STM_OTA_SPI_FLASH_Init(void);

/**
 * @brief  Delete old image in external flash
 */
STM_OTA_StatusTypeDef STM_OTA_SPI_FLASH_Delete_Image(uint32_t Address, uint32_t Length);

/**
 * @brief  Write chunk of data in external flash
 */
STM_OTA_StatusTypeDef STM_OTA_SPI_FLASH_WriteChunk(uint32_t * pDestAddress, uint32_t * pSrcBuffer, uint32_t Length);

/**
 * @brief  Read chunk of data in external flash
 */
STM_OTA_StatusTypeDef STM_OTA_SPI_FLASH_ReadChunk(uint32_t * pSrcAddress, uint32_t * pDestBuffer, uint32_t Length);

#endif /* (OTA_EXTERNAL_FLASH_ENABLE == 1) */

#ifdef __cplusplus
}
#endif

#endif /*STM_OTA_SPI_FLASH_H */
