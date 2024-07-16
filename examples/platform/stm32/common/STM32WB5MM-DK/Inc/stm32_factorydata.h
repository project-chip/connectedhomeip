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
#ifndef STM32_FACTORYDATA_H
#define STM32_FACTORYDATA_H

/* Includes ------------------------------------------------------------------*/
#include "utilities_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PRIVATE_KEY_LEN 32
#define PUBLIC_KEY_LEN 65
typedef enum
{
    DATAFACTORY_OK,
    DATAFACTORY_DATA_NOT_FOUND,
    DATAFACTORY_BUFFER_TOO_SMALL,
    DATAFACTORY_PARAM_ERROR,

} FACTORYDATA_StatusTypeDef;

typedef enum
{
    /* DeviceAttestationCredentialsProvider */
    TAG_ID_CERTIFICATION_DECLARATION                    = 1,
    TAG_ID_FIRMWARE_INFORMATION                         = 2,
    TAG_ID_DEVICE_ATTESTATION_CERTIFICATE               = 3,
    TAG_ID_PRODUCT_ATTESTATION_INTERMEDIATE_CERTIFICATE = 4,
    TAG_ID_DEVICE_ATTESTATION_PRIVATE_KEY               = 5,
    TAG_ID_DEVICE_ATTESTATION_PUBLIC_KEY                = 6,
    /* CommissionableDataProvider */
    TAG_ID_SETUP_DISCRIMINATOR    = 11,
    TAG_ID_SPAKE2_ITERATION_COUNT = 12,
    TAG_ID_SPAKE2_SALT            = 13,
    TAG_ID_SPAKE2_VERIFIER        = 14,
    TAG_ID_SPAKE2_SETUP_PASSCODE  = 15,
    /* DeviceInstanceInfoProvider */
    TAG_ID_VENDOR_NAME             = 21,
    TAG_ID_VENDOR_ID               = 22,
    TAG_ID_PRODUCT_NAME            = 23,
    TAG_ID_PRODUCT_ID              = 24,
    TAG_ID_SERIAL_NUMBER           = 25,
    TAG_ID_MANUFACTURING_DATE      = 26,
    TAG_ID_HARDWARE_VERSION        = 27,
    TAG_ID_HARDWARE_VERSION_STRING = 28,
    TAG_ID_ROTATING_DEVICE_ID      = 29,
    /* Platform specific */
    TAG_ID_ENABLE_KEY = 41,

} FACTORYDATA_TagId;

FACTORYDATA_StatusTypeDef FACTORYDATA_GetValue(FACTORYDATA_TagId tag, uint8_t * data, uint32_t size, uint32_t * out_datalength);

#ifdef __cplusplus
}
#endif
#endif /*STM32_FACTORYDATA_H*/
