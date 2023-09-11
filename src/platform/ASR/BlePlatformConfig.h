/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ==================== Platform Adaptations ====================

#define BLE_CONNECTION_OBJECT uint16_t
#define BLE_CONNECTION_UNINITIALIZED ((uint16_t) 0xFFFF)
#define BLE_MAX_RECEIVE_WINDOW_SIZE 5

#define BLE_CONFIG_ERROR_FORMAT PRId32
#define BLE_CONFIG_ERROR_TYPE uint32_t
#define BLE_CONFIG_NO_ERROR 0
#define BLE_CONFIG_ERROR_MIN 6000000
#define BLE_CONFIG_ERROR_MAX 6000999
#define _BLE_CONFIG_ERROR(e) (BLE_CONFIG_ERROR_MIN + (e))
