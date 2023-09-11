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
 *          Layer on mbed platforms.
 *
 */

#pragma once

// ==================== Platform Adaptations ====================

#define BLE_CONNECTION_OBJECT uintptr_t
#define BLE_CONNECTION_UNINITIALIZED ((uintptr_t) -1)
#define BLE_MAX_RECEIVE_WINDOW_SIZE 5

#define BLE_CONFIG_ERROR_TYPE int32_t
#define BLE_CONFIG_ERROR_FORMAT PRId32

#define BLE_CONFIG_NO_ERROR 0
#define BLE_CONFIG_ERROR_MIN 6000000
#define BLE_CONFIG_ERROR_MAX 6000999
#define _BLE_CONFIG_ERROR(e) (BLE_CONFIG_ERROR_MIN + (e))

// ========== Platform-specific Configuration Overrides =========

/* none so far */
