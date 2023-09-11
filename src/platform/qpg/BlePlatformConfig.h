/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP BLE
 *          Layer on Qorvo QPG platforms.
 *
 */

#pragma once

// ==================== Platform Adaptations ====================

#define BLE_CONNECTION_OBJECT uint16_t
#define BLE_CONNECTION_UNINITIALIZED (0xFFFF)
#define BLE_MAX_RECEIVE_WINDOW_SIZE 5

#define BLE_CONFIG_ERROR_MIN 6000000
#define BLE_CONFIG_ERROR_MAX 6000999

// ========== Platform-specific Configuration Overrides =========

/* none so far */
