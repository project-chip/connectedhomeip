/*
 * Copyright (c) 2023 ASR Microelectronics (Shanghai) Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _ASR_FACTORY_DATA_PARSER_H_
#define _ASR_FACTORY_DATA_PARSER_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER
#define CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER 0
#endif

#ifndef CONFIG_ENABLE_ASR_FACTORY_DEVICE_INFO_PROVIDER
#define CONFIG_ENABLE_ASR_FACTORY_DEVICE_INFO_PROVIDER 0
#endif

#if CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER

#define ASR_CONFIG_BASE PARTITION_MATTER_CONFIG

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

#define DAC_PRIKEY_LEN 32
#define MATTER_FACTORY_VERSION 2
#define MATTER_NO_KEY 0x01

/**
 * @brief Tries to read the factory data within the given partition.
 *
 * @param matter_partition The matter partition to be find out.
 * @param buf Output buffer to store found factory data.
 * @param buf_len Size of buffer.
 * @param outlen Actual size of found factory data.
 * @return true on 0, false otherwise
 */
int32_t asr_factory_config_read(asr_matter_partition_t matter_partition, uint8_t * buf, uint32_t buf_len, uint32_t * out_len);
/**
 * @brief Init and check if the factory data filed is valid.
 *
 * @return true on 0, false otherwise
 */
uint8_t asr_factory_check();
#endif

#ifdef __cplusplus
}
#endif
#endif //_ASR_FACTORY_DATA_PARSER_H_
