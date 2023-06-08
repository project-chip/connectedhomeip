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
#define CHIP_CONFIG_SECURITY_TEST_MODE CONFIG_SECURITY_TEST_MODE

#ifndef CHIP_CONFIG_MAX_FABRICS
#define CHIP_CONFIG_MAX_FABRICS CONFIG_MAX_FABRICS
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
