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
 *          on Silabs platforms using the Silicon Labs SDK.
 */

#pragma once

// ==================== Platform Adaptations ====================

#define CHIP_DEVICE_CONFIG_SILABS_NVM3_ERROR_MIN 0xB00000
#define CHIP_DEVICE_CONFIG_SILABS_BLE_ERROR_MIN 0xC00000

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0

/**
 * CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
 *
 * A string identifying the software version running on the device.
 */
#ifdef SL_MATTER_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING SL_MATTER_VERSION_STRING
#endif

/**
 * CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
 *
 * A monothonic number identifying the software version running on the device.
 */
#ifdef SL_MATTER_VERSION
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION SL_MATTER_VERSION
#endif

/**
 * CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION
 *
 * The hardware version number assigned to device or product by the device vendor.  This
 * number is scoped to the device product id, and typically corresponds to a revision of the
 * physical device, a change to its packaging, and/or a change to its marketing presentation.
 * This value is generally *not* incremented for device software versions.
 */
#ifdef SL_HARDWARE_VERSION
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION SL_HARDWARE_VERSION
#endif

#if defined(SL_WIFI)
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 1
#else
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#if CHIP_ENABLE_OPENTHREAD

#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_COMMISSIONABLE_DISCOVERY 1
#endif /* CHIP_ENABLE_OPENTHREAD */
#endif /* defined(SL_WIFI) */

#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 1

#if defined(SL_WIFI)

#ifndef CHIP_DEVICE_CONFIG_ENABLE_IPV4
#define CHIP_DEVICE_CONFIG_ENABLE_IPV4 0
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

#ifdef CHIP_DEVICE_CONFIG_ENABLE_SED
#define CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL chip::System::Clock::Milliseconds32(300)
#define CHIP_DEVICE_CONFIG_SED_ACTIVE_INTERVAL chip::System::Clock::Milliseconds32(10)
#endif /* CHIP_DEVICE_CONFIG_ENABLE_SED */

#endif /* SL_WIFI */

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to the EFR32 platform.
// These can be overridden by the application as needed.

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
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE (6 * 1024)
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE

#ifndef CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL
#define CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL chip::System::Clock::Milliseconds32(SL_OT_IDLE_INTERVAL)
#endif // CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL

#ifndef CHIP_DEVICE_CONFIG_SED_ACTIVE_INTERVAL
#define CHIP_DEVICE_CONFIG_SED_ACTIVE_INTERVAL chip::System::Clock::Milliseconds32(SL_OT_ACTIVE_INTERVAL)
#endif // CHIP_DEVICE_CONFIG_SED_ACTIVE_INTERVAL

#ifndef CHIP_DEVICE_CONFIG_SED_ACTIVE_THRESHOLD
#define CHIP_DEVICE_CONFIG_SED_ACTIVE_THRESHOLD chip::System::Clock::Milliseconds32(SL_ACTIVE_MODE_THRESHOLD)
#endif // CHIP_DEVICE_CONFIG_SED_ACTIVE_THRESHOLD

#ifndef CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE
#if defined(EFR32MG21)
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE (2 * 1024)
#else
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE (8 * 1024)
#endif
#endif // CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0

#ifndef CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME
#define CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME "Bluetooth event handler"
#endif // CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME

#define CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE 25
