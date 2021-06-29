/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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
 *          mbed platforms.
 */

#pragma once

#include <stdint.h>

// ==================== General Platform Adaptations ====================

#define CHIP_CONFIG_ERROR_TYPE int32_t
#define CHIP_CONFIG_NO_ERROR 0
#define CHIP_CONFIG_ERROR_MIN 4000000
#define CHIP_CONFIG_ERROR_MAX 4000999

#define ASN1_CONFIG_ERROR_TYPE int32_t
#define ASN1_CONFIG_NO_ERROR 0
#define ASN1_CONFIG_ERROR_MIN 5000000
#define ASN1_CONFIG_ERROR_MAX 5000999

#define ChipDie() abort()

#define CHIP_CONFIG_PERSISTED_STORAGE_KEY_TYPE const char *
#define CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID 1
#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 2

#define CHIP_CONFIG_TIME_ENABLE_CLIENT 1
#define CHIP_CONFIG_TIME_ENABLE_SERVER 0

// ==================== Security Adaptations ====================

#define CHIP_CONFIG_USE_OPENSSL_ECC 0
#define CHIP_CONFIG_USE_MICRO_ECC 0

#define CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL 0
#define CHIP_CONFIG_HASH_IMPLEMENTATION_MINCRYPT 1
#define CHIP_CONFIG_HASH_IMPLEMENTATION_MBEDTLS 0
#define CHIP_CONFIG_HASH_IMPLEMENTATION_PLATFORM 0

// FIXME: EFR32 set to MBED-TLS (But this is third-party repo in CHIP, not SDK)

#define CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL 0
#define CHIP_CONFIG_AES_IMPLEMENTATION_AESNI 0
#define CHIP_CONFIG_AES_IMPLEMENTATION_MBEDTLS 1
#define CHIP_CONFIG_AES_IMPLEMENTATION_PLATFORM 0

// FIXME: EFR32 currently set to CHIP (Does this use Entropy.cpp ?)

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

// ==================== General Configuration Overrides ====================

#ifndef CHIP_CONFIG_MAX_PEER_NODES
#define CHIP_CONFIG_MAX_PEER_NODES 16
#endif // CHIP_CONFIG_MAX_PEER_NODES

#ifndef CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS 16
#endif // CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS

#ifndef CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS 8
#endif // CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS

#ifndef CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT
#define CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT 6
#endif // CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT

#ifndef CHIP_LOG_FILTERING
#define CHIP_LOG_FILTERING 0
#endif // CHIP_LOG_FILTERING

#ifndef CHIP_CONFIG_MAX_INTERFACES
#define CHIP_CONFIG_MAX_INTERFACES 4
#endif // CHIP_CONFIG_MAX_INTERFACES

#ifndef CHIP_CONFIG_MAX_LOCAL_ADDR_UDP_ENDPOINTS
#define CHIP_CONFIG_MAX_LOCAL_ADDR_UDP_ENDPOINTS 4
#endif // CHIP_CONFIG_MAX_LOCAL_ADDR_UDP_ENDPOINTS

// ==================== Security Configuration Overrides ====================

#ifndef CHIP_CONFIG_MAX_APPLICATION_GROUPS
#define CHIP_CONFIG_MAX_APPLICATION_GROUPS 4
#endif // CHIP_CONFIG_MAX_APPLICATION_GROUPS

#ifndef CHIP_CONFIG_DEBUG_CERT_VALIDATION
#define CHIP_CONFIG_DEBUG_CERT_VALIDATION 0
#endif // CHIP_CONFIG_DEBUG_CERT_VALIDATION

#ifndef CHIP_CONFIG_ENABLE_CASE_RESPONDER
#define CHIP_CONFIG_ENABLE_CASE_RESPONDER 1
#endif // CHIP_CONFIG_ENABLE_CASE_RESPONDER
