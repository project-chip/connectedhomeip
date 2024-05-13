/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *          on NXP platforms using the NXP SDK.
 */

#pragma once

// ==================== Platform Adaptations ====================

#ifndef CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

#ifndef CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP

#if !CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define CHIP_DEVICE_CONFIG_ENABLE_UNPROVISIONED_MDNS 1
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#ifndef CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 0
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#ifndef CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 0
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#ifndef CHIP_DEVICE_BLE_ADVERTISING_PRIORITY
/// Priority of the Matter BLE advertising when there are multiple application
/// components that compete for the BLE advertising.
#define CHIP_DEVICE_BLE_ADVERTISING_PRIORITY 0
#endif // CHIP_DEVICE_BLE_ADVERTISING_PRIORITY

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to the NXP platform.
// These can be overridden by the application as needed.

// ========== Platform-specific Configuration Overrides =========
#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 10240
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE

#ifndef CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE 8192
#endif // CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE

#ifndef CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY
/* If supported, the Thread task should have a highter priority than the app task */
#define CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY 5
#endif // CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY

#ifndef CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME
#define CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME "BLE App Task"
#endif // CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME

#ifndef CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY

#ifndef CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY

#ifndef CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL

#ifndef CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
#define CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED 1
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED

#ifndef CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART
#define CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART 0
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1
/* Thread DNS client enablement required for OTA */
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 1
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DISCOVERY 1
#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1
#endif
