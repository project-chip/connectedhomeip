/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
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
 *          Layer on the PSoC6 platform.
 *
 */

#pragma once

// ==================== Platform Adaptations ====================

#define BLE_CONNECTION_OBJECT uint16_t
#define BLE_CONNECTION_UNINITIALIZED ((uint16_t) 0xFFFF)
#define BLE_MAX_RECEIVE_WINDOW_SIZE 5

#define BLE_CONFIG_ERROR_FORMAT PRId32
#define BLE_CONFIG_ERROR_TYPE cy_rslt_t
#define BLE_CONFIG_NO_ERROR CY_RSLT_SUCCESS
#define BLE_CONFIG_ERROR_MIN 6000000
#define BLE_CONFIG_ERROR_MAX 6000999
#define _BLE_CONFIG_ERROR(e) (BLE_CONFIG_ERROR_MIN + (e))
