/*
 *
 *    Copyright (c) 2020, 2025 Project CHIP Authors
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
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 1
#endif

#ifndef CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#define CHIP_DEVICE_CHIP0BLE_DEBUG 0
#endif

#define CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC 0
// #define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_GLOBAL_EIDC_KEY 2

/**
 * @def CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID_LENGTH
 *
 * Set unique id to maximum length if not defined to ensure the actual unique
 * id is retrieved instead of the default one (if factory data read fails).
 */
#ifndef CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID_LENGTH
#define CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID_LENGTH 32
#endif

#define CHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER <lib/address_resolve/AddressResolve_DefaultImpl.h>

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

/**
 * @def CHIP_DEVICE_LAYER_OTA_REBOOT_DELAY
 *
 * The delay before rebooting after an OTA process was finished.
 */
#ifndef CHIP_DEVICE_LAYER_OTA_REBOOT_DELAY
#define CHIP_DEVICE_LAYER_OTA_REBOOT_DELAY 3000
#endif // CHIP_DEVICE_LAYER_OTA_REBOOT_DELAY

// ========== Platform-specific Configuration Overrides =========
#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE (6 * 1024)
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE

#ifndef CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE 3072
#endif // CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE

// Max size of event queue
#define CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE 25

#ifndef CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME
#define CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME "BLE App Task"
#endif // CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0

#define CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED 1

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

#ifndef CONFIG_CHIP_LOAD_REAL_FACTORY_DATA
#ifdef CONFIG_CHIP_PLAT_LOAD_REAL_FACTORY_DATA
#define CONFIG_CHIP_LOAD_REAL_FACTORY_DATA CONFIG_CHIP_PLAT_LOAD_REAL_FACTORY_DATA
#else
#define CONFIG_CHIP_LOAD_REAL_FACTORY_DATA 0
#endif /* CONFIG_CHIP_PLAT_LOAD_REAL_FACTORY_DATA */
#endif /* CONFIG_CHIP_LOAD_REAL_FACTORY_DATA */

#if CONFIG_CHIP_LOAD_REAL_FACTORY_DATA
#ifndef CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION
//-> format_version = 1
//-> vendor_id = 0x1037
//-> product_id_array = [ 0xA401 ]
//-> device_type_id = 0x0015
//-> certificate_id = "ZIG20142ZB330003-24"
//-> security_level = 0
//-> security_information = 0
//-> version_number = 0x2694
//-> certification_type = 1
//-> dac_origin_vendor_id is not present
//-> dac_origin_product_id is not present
#define CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION                                                                               \
    {                                                                                                                              \
        0x30, 0x81, 0xe7, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02, 0xa0, 0x81, 0xd9, 0x30, 0x81, 0xd6,    \
            0x02, 0x01, 0x03, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30,      \
            0x44, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01, 0xa0, 0x37, 0x04, 0x35, 0x15, 0x24, 0x00,      \
            0x01, 0x25, 0x01, 0x37, 0x10, 0x36, 0x02, 0x05, 0x01, 0xa4, 0x18, 0x24, 0x03, 0x15, 0x2c, 0x04, 0x13, 0x5a, 0x49,      \
            0x47, 0x32, 0x30, 0x31, 0x34, 0x32, 0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30, 0x33, 0x2d, 0x32, 0x34, 0x24, 0x05,      \
            0x00, 0x24, 0x06, 0x00, 0x25, 0x07, 0x76, 0x98, 0x24, 0x08, 0x01, 0x18, 0x31, 0x7c, 0x30, 0x7a, 0x02, 0x01, 0x03,      \
            0x80, 0x14, 0x62, 0xfa, 0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04,      \
            0xf3, 0x71, 0x60, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0a, 0x06,      \
            0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x04, 0x46, 0x30, 0x44, 0x02, 0x20, 0x12, 0xec, 0x79, 0xdc,      \
            0x03, 0xd3, 0x4f, 0xf9, 0x79, 0xef, 0x56, 0x4e, 0x5b, 0x4f, 0xfc, 0xf5, 0xb1, 0x5a, 0xdb, 0xdf, 0xd9, 0xf8, 0x47,      \
            0xff, 0x81, 0xc3, 0x82, 0x2f, 0xa3, 0x2b, 0xb8, 0x3f, 0x02, 0x20, 0x53, 0x0d, 0x5d, 0xbd, 0xc6, 0xa4, 0x80, 0x67,      \
            0x1f, 0x10, 0xfb, 0xab, 0x00, 0x08, 0xee, 0x15, 0xa0, 0x6c, 0x40, 0x97, 0x55, 0x80, 0x28, 0x3e, 0xf3, 0xd9, 0x61,      \
            0x1f, 0x5b, 0x1d, 0x51, 0x02                                                                                           \
    }

// All remaining data will be pulled from the provisioning region of flash.
#endif // CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION
#endif // CONFIG_CHIP_LOAD_REAL_FACTORY_DATA

// Include default nxp platform configurations
#include "platform/nxp/common/CHIPDeviceNXPPlatformDefaultConfig.h"
