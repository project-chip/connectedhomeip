/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ==================== Platform Adaptations ====================

struct bt_conn;
#define BLE_CONNECTION_OBJECT bt_conn *
#define BLE_CONNECTION_UNINITIALIZED nullptr
#define BLE_MAX_RECEIVE_WINDOW_SIZE 5
