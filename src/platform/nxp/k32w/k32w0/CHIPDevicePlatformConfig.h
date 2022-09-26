/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Google LLC.
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
 *          Platform-specific configuration overrides for the chip Device Layer
 *          on K32W platforms using the NXP SDK.
 */

#pragma once

// ==================== Platform Adaptations ====================

#define K32W_NO_ERRORS 0
#define K32W_ENTRY_NOT_FOUND 1

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0

#ifndef CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#define CHIP_DEVICE_CHIP0BLE_DEBUG 0
#endif

#define CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC 0

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to the K32W platform.
// These can be overridden by the application as needed.

/**
 * @def CHIP_DEVICE_LAYER_BLE_OBSERVER_PRIORITY
 *
 * The priority of the SoftDevice observer event handler registered by the
 * chip BleLayer.
 */
#ifndef CHIP_DEVICE_LAYER_BLE_OBSERVER_PRIORITY
#define CHIP_DEVICE_LAYER_BLE_OBSERVER_PRIORITY 3
#endif // CHIP_DEVICE_LAYER_BLE_OBSERVER_PRIORITY

/**
 * @def CHIP_DEVICE_LAYER_BLE_CONN_CFG_TAG
 *
 * The SoftDevice BLE connection configuration tag used by the chip
 * BleLayer.
 */
#ifndef CHIP_DEVICE_LAYER_BLE_CONN_CFG_TAG
#define CHIP_DEVICE_LAYER_BLE_CONN_CFG_TAG 1
#endif // CHIP_DEVICE_LAYER_BLE_CONN_CFG_TAG

// ========== Platform-specific Configuration Overrides =========
#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE (5 * 1024)
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE

#ifndef CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE 3072
#endif // CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE

// Max size of event queue
#define CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE 75

#ifndef CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME
#define CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME "BLE App Task"
#endif // CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0
#define CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH 0

#define CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART 0

#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 0

#define CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY 3

#define CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY 2

#if CHIP_ENABLE_OPENTHREAD
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 1
#endif

#define CHIP_DEVICE_CONFIG_ENABLE_TEST_SETUP_PARAMS 1
