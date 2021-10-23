/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#define CHIP_LOG_FILTERING 0
#define CHIP_CONFIG_TIME_ENABLE_CLIENT 1
#define CHIP_CONFIG_TIME_ENABLE_SERVER 0

#define ChipDie() abort()

// ==================== Security Adaptations ====================

#define CHIP_CONFIG_USE_OPENSSL_ECC 0
#define CHIP_CONFIG_USE_MICRO_ECC 0

#define CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL 0
#define CHIP_CONFIG_HASH_IMPLEMENTATION_MINCRYPT 1

#define CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL 0
#define CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG 1

#define CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL 0
#define CHIP_CONFIG_AES_IMPLEMENTATION_AESNI 0
#define CHIP_CONFIG_AES_IMPLEMENTATION_PLATFORM 1

#define CHIP_CONFIG_SUPPORT_PASE_CONFIG0 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG1 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG2 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG3 0
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG4 1

#define CHIP_CONFIG_ENABLE_KEY_EXPORT_INITIATOR 0

#define CHIP_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT 0

// ==================== Kconfig Overrides ====================

// The following values are configured via the ESP-IDF Kconfig mechanism.

#define CHIP_CONFIG_MAX_CONNECTIONS CONFIG_MAX_CONNECTIONS
#define CHIP_CONFIG_MAX_PEER_NODES CONFIG_MAX_PEER_NODES
#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS CONFIG_MAX_EXCHANGE_CONTEXTS
#define CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT
#define CHIP_CONFIG_MAX_SESSION_KEYS CONFIG_MAX_SESSION_KEYS
#define CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
#define CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS
#define CHIP_CONFIG_MAX_APPLICATION_EPOCH_KEYS CONFIG_MAX_APPLICATION_EPOCH_KEYS
#define CHIP_CONFIG_MAX_APPLICATION_GROUPS CONFIG_MAX_APPLICATION_GROUPS
#define CHIP_CONFIG_DEFAULT_SECURITY_SESSION_ESTABLISHMENT_TIMEOUT CONFIG_DEFAULT_SECURITY_SESSION_ESTABLISHMENT_TIMEOUT
#define CHIP_CONFIG_DEFAULT_SECURITY_SESSION_IDLE_TIMEOUT CONFIG_DEFAULT_SECURITY_SESSION_IDLE_TIMEOUT
#define CHIP_CONFIG_SECURITY_TEST_MODE CONFIG_SECURITY_TEST_MODE
#define CHIP_CONFIG_REQUIRE_AUTH !CONFIG_DISABLE_PROVISIONING_AUTH
#define CHIP_CONFIG_DEBUG_CERT_VALIDATION CONFIG_DEBUG_CERT_VALIDATION
#define CHIP_CONFIG_ENABLE_PASE_INITIATOR CONFIG_ENABLE_PASE_INITIATOR
#define CHIP_CONFIG_ENABLE_PASE_RESPONDER CONFIG_ENABLE_PASE_RESPONDER
#define CHIP_CONFIG_ENABLE_CASE_INITIATOR CONFIG_ENABLE_CASE_INITIATOR
#define CHIP_CONFIG_ENABLE_CASE_RESPONDER CONFIG_ENABLE_CASE_RESPONDER
#define CHIP_CONFIG_DEFAULT_INCOMING_CONNECTION_IDLE_TIMEOUT CONFIG_DEFAULT_INCOMING_CONNECTION_IDLE_TIMEOUT
