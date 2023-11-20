/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#pragma once

#include "stdint.h"
#include "string.h"
#include "tlv_decode.h"
#ifdef CFG_PLF_RV32
#include "asr_flash.h"
#include "asr_rv32.h"
#elif defined CFG_PLF_DUET
#include "duet_cm4.h"
#include "duet_flash.h"
#define asr_flash_read duet_flash_read
#define asr_factory_decrypt_dac_prvkey duet_factory_decrypt_dac_prvkey
#else
#include "lega_cm4.h"
#include "lega_flash.h"
#define asr_flash_read lega_flash_read
#endif

#ifndef CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER
#define CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER 0
#endif // CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER

#ifndef CONFIG_ENABLE_ASR_FACTORY_DEVICE_INFO_PROVIDER
#define CONFIG_ENABLE_ASR_FACTORY_DEVICE_INFO_PROVIDER 0
#endif // CONFIG_ENABLE_ASR_FACTORY_DEVICE_INFO_PROVIDER

#define ASR_CONFIG_BASE PARTITION_MATTER_CONFIG
#define ASR_MATTER_FACTORY_VERSION 2
#define ASR_CONFIG_MATTER_NO_KEY 0x01

#if CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER
typedef enum ASR_MATTER_PARTITION_T
{
    ASR_VERSION_PARTITION = 0x00,
    ASR_CONFIG_PARTITION,
    ASR_ITERATION_COUNT_PARTITION,
    ASR_SALT_PARTITION,
    ASR_VERIFIER_PARTITION,
    ASR_DISCRIMINATOR_PARTITION,
    ASR_DAC_CERT_PARTITION,
    ASR_DAC_KEY_PARTITION,
    ASR_DAC_PUB_KEY_PARTITION,
    ASR_PAI_CERT_PARTITION,
    ASR_CERT_DCLRN_PARTITION,
    ASR_CHIP_ID_PARTITION,
#if CONFIG_ENABLE_ASR_FACTORY_DEVICE_INFO_PROVIDER
    ASR_VENDOR_NAME_PARTITION,
    ASR_VENDOR_ID_PARTITION,
    ASR_PRODUCT_NAME_PARTITION,
    ASR_PRODUCT_ID_PARTITION,
    ASR_ROTATING_UNIQUE_ID_PARTITION,
    ASR_MANUFACTURY_DATE_PARTITION,
    ASR_SERIAL_NUMBER_PARTITION,
    ASR_HARDWARE_VERSION_PARTITION,
    ASR_HARDWARE_VERSION_STR_PARTITION,
    ASR_PRODUCT_URL_PARTITION,
    ASR_PRODUCT_LABEL_PARTITION,
    ASR_PART_NUMBER_PARTITION,
#endif
    ASR_MATTER_PARTITION_MAX,
} asr_matter_partition_t;

typedef struct
{
    const char * partition_name;
} matter_partition_t;

typedef enum
{
    FACTORY_NO_ERROR = 0x00,
    FACTORY_BUFFER_TOO_SMALL,
    FACTORY_VALUE_NOT_FOUND,
    FACTORY_INVALID_INPUT,
    FACTORY_NOT_SUPPORTED,
    FACTORY_VERSION_MISMATCH,
    FACTORY_DATA_CHECK_FAILED,
    FACTORY_DATA_INIT_FAILED,
} factory_error_t;

/**
 * @brief Tries to read the factory data within the given partition.
 *
 * @param matter_partition The matter partition to be find out.
 * @param buf Output buffer to store found factory data.
 * @param buf_len Size of buffer.
 * @param outlen Actual size of found factory data.
 * @return FACTORY_NO_ERROR on success, other factory_error_t values from implementation on other errors.
 */
factory_error_t asr_factory_config_read(asr_matter_partition_t matter_partition, uint8_t * buf, uint32_t buf_len,
                                        uint32_t * out_len);
/**
 * @brief Init and check if the factory data filed is valid.
 *
 * @return FACTORY_NO_ERROR on success, other factory_error_t values from implementation on other errors.
 */
factory_error_t asr_factory_check();

#endif // CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER
