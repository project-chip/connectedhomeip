/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Platform-specific configuration overrides for the Chip Device Layer
 *          on BL602 platforms using the BouffaloLab SDK.
 */

#pragma once

// ==================== Platform Adaptations ====================

#define CHIP_DEVICE_CONFIG_BL602_EASYFLASH_ERROR_MIN 22000000
#define CHIP_DEVICE_CONFIG_BL602_BLE_ERROR_MIN 23000000

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0

#if CHIP_ENABLE_OPENTHREAD
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 1
#define CHIP_DEVICE_CONFIG_ENABLE_MDNS 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1
#endif

//#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 1

#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_CRIT_EIDC_KEY 2
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_PROD_EIDC_KEY 3
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_INFO_EIDC_KEY 4
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_DEBUG_EIDC_KEY 5

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to the BL602 platform.
// These can be overridden by the application as needed.

// -------------- BL602 EasyFlash Storage Configuration -------------

/**
 *  @def CHIP_DEVICE_CONFIG_EASYFLASH_MAX_NUM_OBJECTS
 *
 *  @brief
 *    Configures the size of the easyflash cache and should be set >= the
 *    maximum number of Chip Config objects, e.g...
 *    Factory configs[5], System configs[23], Counter configs[32] + margin[4] = 64.
 *
 */
#ifndef CHIP_DEVICE_CONFIG_EASYFLASH_MAX_NUM_OBJECTS
#define CHIP_DEVICE_CONFIG_EASYFLASH_MAX_NUM_OBJECTS 64
#endif // CHIP_DEVICE_CONFIG_EASYFLASH_MAX_NUM_OBJECTS

/**
 *  @def CHIP_DEVICE_CONFIG_EASYFLASH_MAX_OBJECT_SIZE
 *
 *  @brief
 *    This determines the max size for any Chip easyflash object
 *    (e.g. for Config 'string' or 'binary' types).
 */
#ifndef CHIP_DEVICE_CONFIG_EASYFLASH_MAX_OBJECT_SIZE
#define CHIP_DEVICE_CONFIG_EASYFLASH_MAX_OBJECT_SIZE 1000
#endif // CHIP_DEVICE_CONFIG_EASYFLASH_MAX_OBJECT_SIZE

/**
 *  @def CHIP_DEVICE_CONFIG_EASYFLASH_NUM_FLASH_PAGES_FOR_STORAGE
 *
 *  @brief
 *    This determines the Flash size used for easyflash data storage:-
 *    (assuming 2k Flash page size) => Total Flash size for easyflash: 8 * 2k = 16k
 *    The total size should allow sufficient margin for wear-levelling and
 *    repacking.
 */
#ifndef CHIP_DEVICE_CONFIG_EASYFLASH_NUM_FLASH_PAGES_FOR_STORAGE
#define CHIP_DEVICE_CONFIG_EASYFLASH_NUM_FLASH_PAGES_FOR_STORAGE 8
#endif // CHIP_DEVICE_CONFIG_EASYFLASH_NUM_FLASH_PAGES_FOR_STORAGE

// ========== Platform-specific Configuration Overrides =========

#ifndef CHIP_DEVICE_CONFIG_BLE_LL_TASK_PRIORITY
#define CHIP_DEVICE_CONFIG_BLE_LL_TASK_PRIORITY (configTIMER_TASK_PRIORITY - 1)
#endif // CHIP_DEVICE_CONFIG_BLE_LL_TASK_PRIORITY

#ifndef CHIP_DEVICE_CONFIG_BLE_STACK_TASK_PRIORITY
#define CHIP_DEVICE_CONFIG_BLE_STACK_TASK_PRIORITY (CHIP_DEVICE_CONFIG_BLE_LL_TASK_PRIORITY - 1)
#endif // CHIP_DEVICE_CONFIG_BLE_STACK_TASK_PRIORITY

#ifndef CHIP_DEVICE_CONFIG_BLE_APP_TASK_PRIORITY
#define CHIP_DEVICE_CONFIG_BLE_APP_TASK_PRIORITY (CHIP_DEVICE_CONFIG_BLE_STACK_TASK_PRIORITY - 1)
#endif // CHIP_DEVICE_CONFIG_BLE_STACK_TASK_PRIORITY

#ifndef CHIP_DEVICE_CONFIG_BLE_APP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_BLE_APP_TASK_STACK_SIZE 1536
#endif // CHIP_DEVICE_CONFIG_BLE_APP_TASK_STACK_SIZE

#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE (8 * 1024)
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE

#ifndef CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE
#if defined(BL602MG21)
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE (2 * 1024)
#else
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE (3 * 1024)
#endif
#endif // CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0

#ifndef CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME
#define CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME "BLE_EVENT"
#endif // CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME

#define CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE 25
