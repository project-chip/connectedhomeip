/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Platform-specific configuration overrides for the chip Device Layer
 *          on Linux platforms.
 */

#pragma once

// ==================== Platform Adaptations ====================

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 1
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 1

#ifndef CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD CHIP_ENABLE_OPENTHREAD
#endif

#ifndef CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 0
#endif

#define CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC 0

#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_CRIT_EIDC_KEY 2
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_PROD_EIDC_KEY 3
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_INFO_EIDC_KEY 4
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_DEBUG_EIDC_KEY 5

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DISCOVERY 1

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to Linux platforms.
// These can be overridden by the application as needed.

/**
 * @def CHIP_DEVICE_LAYER_BLE_OBSERVER_PRIORITY
 *
 * The priority of the SoftDevice observer event handler registered by the
 * CHIP BleLayer.
 */
#ifndef CHIP_DEVICE_LAYER_BLE_OBSERVER_PRIORITY
#define CHIP_DEVICE_LAYER_BLE_OBSERVER_PRIORITY 3
#endif // CHIP_DEVICE_LAYER_BLE_OBSERVER_PRIORITY

/**
 * @def CHIP_DEVICE_LAYER_BLE_CONN_CFG_TAG
 *
 * The SoftDevice BLE connection configuration tag used by the CHIP
 * BleLayer.
 */
#ifndef CHIP_DEVICE_LAYER_BLE_CONN_CFG_TAG
#define CHIP_DEVICE_LAYER_BLE_CONN_CFG_TAG 1
#endif // CHIP_DEVICE_LAYER_BLE_CONN_CFG_TAG

// ========== Platform-specific Configuration Overrides =========

#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 8192
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE

#ifndef CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE 8192
#endif // CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0

// TODO: CHIP has redesigned the crypto interface, pending on the final version of CHIP HASH APIs
#define CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH 0
