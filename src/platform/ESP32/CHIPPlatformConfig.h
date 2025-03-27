/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *    @file
 *          Platform-specific configuration overrides for CHIP on
 *          the ESP32 platform.
 */

#pragma once

/* Force sdkconfig to be added as a dependecy. That file is also included as a sideffect of
 * esp_err.h -> assert.h -> sdkconfig.h however since esp_err.h and above are -isystem includes,
 * they are not added as dependencies by GN build systems.
 *
 * This triggers a rebuild of files including CHIPPlatformConfig if sdkconfig.h changes.
 */
#include <sdkconfig.h>

#include "esp_err.h"

// ==================== General Platform Adaptations ====================

#define CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID "enc-msg-counter"
// The ESP NVS implementation limits key names to 15 characters.
#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 15

#ifndef CONFIG_CHIP_LOG_FILTERING
#define CHIP_LOG_FILTERING 0
#else
#define CHIP_LOG_FILTERING CONFIG_CHIP_LOG_FILTERING
#endif

#define CHIP_CONFIG_ABORT() abort()

// ==================== Security Adaptations ====================

// ==================== Kconfig Overrides ====================

// The following values are configured via the ESP-IDF Kconfig mechanism.

#define CHIP_DISPATCH_EVENT_LONG_DISPATCH_TIME_WARNING_THRESHOLD_MS CONFIG_DISPATCH_EVENT_LONG_DISPATCH_TIME_WARNING_THRESHOLD_MS
#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS CONFIG_MAX_EXCHANGE_CONTEXTS

#ifdef CONFIG_SECURITY_TEST_MODE
#define CHIP_CONFIG_SECURITY_TEST_MODE CONFIG_SECURITY_TEST_MODE
#else
#define CHIP_CONFIG_SECURITY_TEST_MODE 0
#endif // CONFIG_SECURITY_TEST_MODE

#ifndef CHIP_CONFIG_MAX_FABRICS
#define CHIP_CONFIG_MAX_FABRICS CONFIG_MAX_FABRICS
#endif

#ifndef CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC
#define CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC
#endif

#ifndef CHIP_CONFIG_MAX_GROUPS_PER_FABRIC
#define CHIP_CONFIG_MAX_GROUPS_PER_FABRIC (CONFIG_MAX_GROUPS_PER_FABRIC_PER_ENDPOINT * CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC)
#endif

#ifndef CHIP_CONFIG_MAX_GROUP_KEYS_PER_FABRIC
#define CHIP_CONFIG_MAX_GROUP_KEYS_PER_FABRIC CONFIG_MAX_GROUP_KEYS_PER_FABRIC
#endif

#ifdef CONFIG_ENABLE_WIFI_STATION
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 1
#else
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#endif

#ifdef CONFIG_ENABLE_WIFI_AP
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 1
#else
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0
#endif

#ifdef CONFIG_ENABLE_ICD_SERVER

#ifndef CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC
#define CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC CONFIG_ICD_IDLE_MODE_INTERVAL_SEC
#endif // CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC

#ifndef CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS
#define CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS CONFIG_ICD_ACTIVE_MODE_INTERVAL_MS
#endif // CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS

#ifndef CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS
#define CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS
#endif // CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS

#ifdef CONFIG_ENABLE_ICD_CIP
#ifndef CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC
#define CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC
#endif // CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC
#endif // CONFIG_ENABLE_ICD_CIP

#endif // CONFIG_ENABLE_ICD_SERVER

#ifdef CONFIG_CHIP_ENABLE_BDX_LOG_TRANSFER
#define CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER 1
#endif // CONFIG_CHIP_ENABLE_BDX_LOG_TRANSFER

#ifdef CONFIG_BUILD_CHIP_TESTS
// CHIP tests use APIs that are declared when CONFIG_BUILD_FOR_HOST_UNIT_TEST is enabled
// Enable CONFIG_BUILD_FOR_HOST_UNIT_TEST when building CHIP test binaries
#define CONFIG_BUILD_FOR_HOST_UNIT_TEST 1
#endif

#ifndef CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
#if CHIP_ENABLE_OPENTHREAD
#define CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL                                                                                \
    chip::System::Clock::Milliseconds32(CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL_FOR_THREAD)
#else
#define CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL                                                                                \
    chip::System::Clock::Milliseconds32(CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL_FOR_WIFI_ETHERNET)
#endif // CHIP_ENABLE_OPENTHREAD
#endif // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL

#ifndef CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
#if CHIP_ENABLE_OPENTHREAD
#define CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL                                                                                  \
    chip::System::Clock::Milliseconds32(CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL_FOR_THREAD)
#else
#define CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL                                                                                  \
    chip::System::Clock::Milliseconds32(CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL_FOR_WIFI_ETHERNET)
#endif // CHIP_ENABLE_OPENTHREAD
#endif // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL

#ifndef CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST
#if CHIP_ENABLE_OPENTHREAD
#define CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST                                                                                \
    chip::System::Clock::Milliseconds32(CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST_FOR_THREAD)
#else
#define CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST                                                                                \
    chip::System::Clock::Milliseconds32(CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST_FOR_WIFI_ETHERNET)
#endif // CHIP_ENABLE_OPENTHREAD
#endif // CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST

#ifndef CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS
#define CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS CONFIG_MRP_MAX_RETRANS
#endif // CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS

#ifdef CONFIG_CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
#define CHIP_SYSTEM_CONFIG_POOL_USE_HEAP 1
#else
#define CHIP_SYSTEM_CONFIG_POOL_USE_HEAP 0
#endif
