/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2020 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP BLE
 *          Layer on K32W platforms using the NXP SDK.
 *
 */

#pragma once

// ==================== Platform Adaptations ====================

#define BLE_CONNECTION_OBJECT uint8_t
#define BLE_CONNECTION_UNINITIALIZED ((uint8_t) 0xFF)

// ========== Platform-specific Configuration Overrides =========

/* none so far */
