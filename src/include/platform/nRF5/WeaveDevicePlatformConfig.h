/*
 *
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
 *          Platform-specific configuration overrides for the Weave Device Layer
 *          on nRF52 platforms using the Nordic nRF5 SDK.
 */


#ifndef WEAVE_DEVICE_PLATFORM_CONFIG_H
#define WEAVE_DEVICE_PLATFORM_CONFIG_H

// ==================== Platform Adaptations ====================

#define WEAVE_DEVICE_CONFIG_NRF5_ERROR_MIN 1
#define WEAVE_DEVICE_CONFIG_NRF5_ERROR_MAX 1000000

#define WEAVE_DEVICE_CONFIG_NRF5_FDS_ERROR_MIN 10000000
#define WEAVE_DEVICE_CONFIG_NRF5_FDS_ERROR_MAX 10000999

#define WEAVE_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define WEAVE_DEVICE_CONFIG_ENABLE_WIFI_AP 0

#define WEAVE_DEVICE_CONFIG_ENABLE_THREAD 1

#define WEAVE_DEVICE_CONFIG_ENABLE_WOBLE 1

#define WEAVE_DEVICE_CONFIG_ENABLE_WEAVE_TIME_SERVICE_TIME_SYNC 1
#define WEAVE_DEVICE_CONFIG_ENABLE_SERVICE_DIRECTORY_TIME_SYNC 0

#define WEAVE_DEVICE_CONFIG_PERSISTED_STORAGE_CRIT_EIDC_KEY 2
#define WEAVE_DEVICE_CONFIG_PERSISTED_STORAGE_PROD_EIDC_KEY 3
#define WEAVE_DEVICE_CONFIG_PERSISTED_STORAGE_INFO_EIDC_KEY 4
#define WEAVE_DEVICE_CONFIG_PERSISTED_STORAGE_DEBUG_EIDC_KEY 5

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to the nRF52 platform.
// These can be overridden by the application as needed.

/**
 * @def WEAVE_DEVICE_LAYER_BLE_OBSERVER_PRIORITY
 *
 * The priority of the SoftDevice observer event handler registered by the
 * OpenWeave BleLayer.
 */
#ifndef WEAVE_DEVICE_LAYER_BLE_OBSERVER_PRIORITY
#define WEAVE_DEVICE_LAYER_BLE_OBSERVER_PRIORITY 3
#endif // WEAVE_DEVICE_LAYER_BLE_OBSERVER_PRIORITY

/**
 * @def WEAVE_DEVICE_LAYER_BLE_CONN_CFG_TAG
 *
 * The SoftDevice BLE connection configuration tag used by the OpenWeave
 * BleLayer.
 */
#ifndef WEAVE_DEVICE_LAYER_BLE_CONN_CFG_TAG
#define WEAVE_DEVICE_LAYER_BLE_CONN_CFG_TAG 1
#endif // WEAVE_DEVICE_LAYER_BLE_CONN_CFG_TAG

// ========== Platform-specific Configuration Overrides =========

#ifndef WEAVE_DEVICE_CONFIG_WEAVE_TASK_STACK_SIZE
#define WEAVE_DEVICE_CONFIG_WEAVE_TASK_STACK_SIZE 8192
#endif // WEAVE_DEVICE_CONFIG_WEAVE_TASK_STACK_SIZE

#ifndef WEAVE_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE
#define WEAVE_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE 8192
#endif // WEAVE_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE

#define WEAVE_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define WEAVE_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 1
#define WEAVE_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0
#define WEAVE_DEVICE_CONFIG_ENABLE_TUNNEL_TELEMETRY 0

#endif // WEAVE_DEVICE_PLATFORM_CONFIG_H
