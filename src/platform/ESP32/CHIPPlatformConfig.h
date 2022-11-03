/*
 * SPDX-FileCopyrightText: (c) 2020-2022 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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

#define CHIP_CONFIG_ABORT() abort()

// ==================== Security Adaptations ====================

// ==================== Kconfig Overrides ====================

// The following values are configured via the ESP-IDF Kconfig mechanism.

#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS CONFIG_MAX_EXCHANGE_CONTEXTS
#define CHIP_CONFIG_MAX_FABRICS CONFIG_MAX_FABRICS
#define CHIP_CONFIG_SECURITY_TEST_MODE CONFIG_SECURITY_TEST_MODE

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
