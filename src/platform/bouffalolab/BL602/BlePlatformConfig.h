/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP BLE
 *          Layer on the BL602 platform.
 *
 */

#pragma once

// ==================== Platform Adaptations ====================

struct bt_conn;
#define BLE_CONNECTION_OBJECT bt_conn *
#define BLE_CONNECTION_UNINITIALIZED nullptr
#define BLE_MAX_RECEIVE_WINDOW_SIZE 5

// ========== Platform-specific Configuration Overrides =========

/* none so far */
