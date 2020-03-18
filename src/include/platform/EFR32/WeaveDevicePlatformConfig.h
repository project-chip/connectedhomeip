/*
 *
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          on EFR32 platforms using the Silicon Labs SDK.
 */

#ifndef WEAVE_DEVICE_PLATFORM_CONFIG_H
#define WEAVE_DEVICE_PLATFORM_CONFIG_H

// ==================== Platform Adaptations ====================

#define WEAVE_DEVICE_CONFIG_EFR32_NVM3_ERROR_MIN 12000000
#define WEAVE_DEVICE_CONFIG_EFR32_BLE_ERROR_MIN 13000000

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

// These are configuration options that are unique to the EFR32 platform.
// These can be overridden by the application as needed.
/**
 * @def WEAVE_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE
 *
 * The maximum size of any log message.
 */
#ifndef WEAVE_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE
#define WEAVE_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE 150
#endif // WEAVE_DEVICE_CONFIG_LOG_MESSAGE_MAX_SIZE

// -------------- EFR32 NVM3 Storage Configuration -------------

/**
 *  @def WEAVE_DEVICE_CONFIG_NVM3_MAX_NUM_OBJECTS
 *
 *  @brief
 *    Configures the size of the nvm3 cache and should be set >= the
 *    maximum number of Weave Config objects, e.g...
 *    Factory configs[5], System configs[23], Counter configs[32] + margin[4] = 64.
 *
 */
#ifndef WEAVE_DEVICE_CONFIG_NVM3_MAX_NUM_OBJECTS
#define WEAVE_DEVICE_CONFIG_NVM3_MAX_NUM_OBJECTS 64
#endif // WEAVE_DEVICE_CONFIG_NVM3_MAX_NUM_OBJECTS

/**
 *  @def WEAVE_DEVICE_CONFIG_NVM3_MAX_OBJECT_SIZE
 *
 *  @brief
 *    This determines the max size for any Weave nvm3 object
 *    (e.g. for Config 'string' or 'binary' types).
 */
#ifndef WEAVE_DEVICE_CONFIG_NVM3_MAX_OBJECT_SIZE
#define WEAVE_DEVICE_CONFIG_NVM3_MAX_OBJECT_SIZE 1000
#endif // WEAVE_DEVICE_CONFIG_NVM3_MAX_OBJECT_SIZE

/**
 *  @def WEAVE_DEVICE_CONFIG_NVM3_NUM_FLASH_PAGES_FOR_STORAGE
 *
 *  @brief
 *    This determines the Flash size used for nvm3 data storage:-
 *    (assuming 2k Flash page size) => Total Flash size for nvm3: 8 * 2k = 16k
 *    The total size should allow sufficient margin for wear-levelling and
 *    repacking.
 */
#ifndef WEAVE_DEVICE_CONFIG_NVM3_NUM_FLASH_PAGES_FOR_STORAGE
#define WEAVE_DEVICE_CONFIG_NVM3_NUM_FLASH_PAGES_FOR_STORAGE 8
#endif // WEAVE_DEVICE_CONFIG_NVM3_NUM_FLASH_PAGES_FOR_STORAGE

// ========== Platform-specific Configuration Overrides =========

#ifndef WEAVE_DEVICE_CONFIG_BLE_LL_TASK_PRIORITY
#define WEAVE_DEVICE_CONFIG_BLE_LL_TASK_PRIORITY (configTIMER_TASK_PRIORITY - 1)
#endif // WEAVE_DEVICE_CONFIG_BLE_LL_TASK_PRIORITY

#ifndef WEAVE_DEVICE_CONFIG_BLE_STACK_TASK_PRIORITY
#define WEAVE_DEVICE_CONFIG_BLE_STACK_TASK_PRIORITY (WEAVE_DEVICE_CONFIG_BLE_LL_TASK_PRIORITY - 1)
#endif // WEAVE_DEVICE_CONFIG_BLE_STACK_TASK_PRIORITY

#ifndef WEAVE_DEVICE_CONFIG_BLE_APP_TASK_PRIORITY
#define WEAVE_DEVICE_CONFIG_BLE_APP_TASK_PRIORITY (WEAVE_DEVICE_CONFIG_BLE_STACK_TASK_PRIORITY - 1)
#endif // WEAVE_DEVICE_CONFIG_BLE_STACK_TASK_PRIORITY

#ifndef WEAVE_DEVICE_CONFIG_BLE_APP_TASK_STACK_SIZE
#define WEAVE_DEVICE_CONFIG_BLE_APP_TASK_STACK_SIZE 1024
#endif // WEAVE_DEVICE_CONFIG_BLE_APP_TASK_STACK_SIZE

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

#ifndef WEAVE_DEVICE_CONFIG_BLE_APP_TASK_NAME
#define WEAVE_DEVICE_CONFIG_BLE_APP_TASK_NAME "Bluetooth App Task"
#endif // WEAVE_DEVICE_CONFIG_BLE_APP_TASK_NAME

#endif // WEAVE_DEVICE_PLATFORM_CONFIG_H
