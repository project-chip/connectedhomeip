/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *          nRF Connect SDK platform.
 */

#pragma once

#ifdef CONFIG_CHIP_CRYPTO_PSA
#include <psa/crypto.h>
#endif

// ==================== General Platform Adaptations ====================

#define CHIP_CONFIG_ABORT() abort()

#define CHIP_CONFIG_PERSISTED_STORAGE_KEY_TYPE const char *
#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 2

#define CHIP_CONFIG_LIFETIIME_PERSISTED_COUNTER_KEY "rc"

// ==================== Security Adaptations ====================

#ifdef CONFIG_CHIP_CRYPTO_PSA
#define CHIP_CONFIG_SHA256_CONTEXT_SIZE sizeof(psa_hash_operation_t)
#elif defined(CONFIG_CC3XX_BACKEND)
// Size of the statically allocated context for SHA256 operations in CryptoPAL
// determined empirically.
#define CHIP_CONFIG_SHA256_CONTEXT_SIZE 244
#else
#define CHIP_CONFIG_SHA256_CONTEXT_SIZE 208
#endif

// ==================== General Configuration Overrides ====================

#ifndef CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS 8
#endif // CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS

#ifndef CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS 8
#endif // CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS

#ifndef CHIP_LOG_FILTERING
#define CHIP_LOG_FILTERING 0
#endif // CHIP_LOG_FILTERING

#ifndef CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS
#define CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS 1
#endif // CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS

#ifndef CHIP_CONFIG_MAX_FABRICS
#define CHIP_CONFIG_MAX_FABRICS 5
#endif

#if CONFIG_CHIP_LOG_SIZE_OPTIMIZATION
// Disable some of the too detailed log modules to save flash
#define CHIP_CONFIG_LOG_MODULE_ExchangeManager_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_Crypto_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_Crypto_PROGRESS 0
#define CHIP_CONFIG_LOG_MODULE_BDX_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_BDX_PROGRESS 0
#define CHIP_CONFIG_LOG_MODULE_EventLogging_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_EventLogging_PROGRESS 0
#define CHIP_CONFIG_LOG_MODULE_SetupPayload_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_SetupPayload_PROGRESS 0
#define CHIP_CONFIG_LOG_MODULE_CASESessionManager_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_CASESessionManager_PROGRESS 0
#define CHIP_CONFIG_LOG_MODULE_DataManagement_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_FabricProvisioning_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_chipSystemLayer_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_chipSystemLayer_PROGRESS 0
#define CHIP_CONFIG_LOG_MODULE_Zcl_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_SecureChannel_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_Ble_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_AppServer_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_Support_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_Support_PROGRESS 0
#endif

// Set MRP retry intervals for Thread and Wi-Fi to test-proven values.
#ifndef CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
#if CHIP_ENABLE_OPENTHREAD
#define CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL (800_ms32)
#else
#define CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL (1000_ms32)
#endif // CHIP_ENABLE_OPENTHREAD
#endif // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL

#ifndef CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
#if CHIP_ENABLE_OPENTHREAD
#define CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL (800_ms32)
#else
#define CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL (1000_ms32)
#endif // CHIP_ENABLE_OPENTHREAD
#endif // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
