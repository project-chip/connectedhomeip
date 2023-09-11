/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP BLE
 *          Layer on the ESP32 platform.
 *
 */

#pragma once

#include "esp_err.h"

// ==================== Platform Adaptations ====================

#define BLE_CONNECTION_OBJECT uint16_t
#define BLE_CONNECTION_UNINITIALIZED ((uint16_t) 0xFFFF)
#define BLE_MAX_RECEIVE_WINDOW_SIZE 5

#define BLE_CONFIG_ERROR_MIN 6000000
#define BLE_CONFIG_ERROR_MAX 6000999
