/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *    @file
 *          Platform-specific configuration overrides for CHIP on
 *          the Texas Instruments CC32XX platform.
 *
 */

#ifndef CHIP_PLATFORM_CONFIG_H
#define CHIP_PLATFORM_CONFIG_H

#include <stdint.h>

// ==================== General Platform Adaptations ====================

// XXX: Seth is there a platform specific error we can use??
#define CHIP_CONFIG_ERROR_TYPE uint32_t
#define CHIP_CONFIG_NO_ERROR (0)

// XXX: Seth is there a platform specific error we can use??
#define ASN1_CONFIG_ERROR_TYPE uint32_t
#define ASN1_CONFIG_NO_ERROR (0)

// XXX: Seth is there a platform specific version that would work better
#define ChipDie() assert()

#define CHIP_CONFIG_PERSISTED_STORAGE_KEY_TYPE char *
#define CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID 1
#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 2

#define CHIP_CONFIG_PERSISTED_STORAGE_KEY_GLOBAL_MESSAGE_COUNTER (char *) "GlobalMCTR"

// ==================== Security Adaptations ====================

// XXX: Seth, how much of this can we move to mbedtls??
#define CHIP_CONFIG_USE_OPENSSL_ECC 0
#define CHIP_CONFIG_USE_MICRO_ECC 1

#define CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL 0
#define CHIP_CONFIG_HASH_IMPLEMENTATION_MINCRYPT 1
#define CHIP_CONFIG_HASH_IMPLEMENTATION_MBEDTLS 0
#define CHIP_CONFIG_HASH_IMPLEMENTATION_PLATFORM 0

#define CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL 0
#define CHIP_CONFIG_AES_IMPLEMENTATION_AESNI 0
#define CHIP_CONFIG_AES_IMPLEMENTATION_MBEDTLS 1
#define CHIP_CONFIG_AES_IMPLEMENTATION_PLATFORM 0

#define CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL 0
#define CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG 1
#define CHIP_CONFIG_RNG_IMPLEMENTATION_PLATFORM 0

#define CHIP_CONFIG_ENABLE_PASE_INITIATOR 0
#define CHIP_CONFIG_ENABLE_PASE_RESPONDER 1
#define CHIP_CONFIG_ENABLE_CASE_INITIATOR 1

#define CHIP_CONFIG_SUPPORT_PASE_CONFIG0 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG1 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG2 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG3 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG4 1

#define CHIP_CONFIG_ENABLE_KEY_EXPORT_INITIATOR 0

#define CHIP_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT 0

#define CHIP_CONFIG_ENABLE_SERVER_IM_EVENT 1

// ==================== General Configuration Overrides ====================

/* none yet */

#endif /* CHIP_PLATFORM_CONFIG_H */
