/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP BLE
 *          Layer on Telink platform.
 *
 */

#pragma once

#include <zephyr/bluetooth/bluetooth.h>

// ==================== Platform Adaptations ====================

#define BLE_CONNECTION_OBJECT bt_conn *
#define BLE_CONNECTION_UNINITIALIZED nullptr
#define BLE_MAX_RECEIVE_WINDOW_SIZE 5

// ========== Platform-specific Configuration Overrides =========

/* none so far */
