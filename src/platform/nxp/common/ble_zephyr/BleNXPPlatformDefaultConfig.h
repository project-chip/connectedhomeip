/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *          Platform-specific configuration overrides for the CHIP BLE
 *          Layer on NXP platforms using the NXP SDK.
 *
 */

#pragma once

// ==================== Platform Adaptations ====================

struct bt_conn;

#ifndef BLE_CONNECTION_OBJECT
#define BLE_CONNECTION_OBJECT bt_conn *
#endif // BLE_CONNECTION_OBJECT

#ifndef BLE_CONNECTION_UNINITIALIZED
#define BLE_CONNECTION_UNINITIALIZED nullptr
#endif // BLE_CONNECTION_UNINITIALIZED

#ifndef BLE_MAX_RECEIVE_WINDOW_SIZE
#define BLE_MAX_RECEIVE_WINDOW_SIZE 5
#endif // BLE_MAX_RECEIVE_WINDOW_SIZE

// ========== Platform-specific Configuration Overrides =========

#ifndef LOG_MODULE_DECLARE
#define LOG_MODULE_DECLARE(...)
#endif // LOG_MODULE_DECLARE

#ifndef LOG_HEXDUMP_DBG
#define LOG_HEXDUMP_DBG(...)
#endif // LOG_HEXDUMP_DBG

/* none so far */
