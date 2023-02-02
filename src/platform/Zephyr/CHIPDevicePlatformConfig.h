/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *          on Zephyr platform.
 */

#pragma once

#include "autoconf.h"

// ==================== Platform Adaptations ====================

#ifndef CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID CONFIG_CHIP_DEVICE_VENDOR_ID
#endif

#ifndef CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID CONFIG_CHIP_DEVICE_PRODUCT_ID
#endif

#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION CONFIG_CHIP_DEVICE_SOFTWARE_VERSION
#endif

#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING
#endif

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0

#define CHIP_DEVICE_CONFIG_ENABLE_THREAD CONFIG_NET_L2_OPENTHREAD

#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE CONFIG_BT

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to Zephyr platforms.
// These can be overridden by the application as needed.

#ifndef CHIP_DEVICE_CONFIG_SETTINGS_KEY
/// Key for all Matter persistent data stored using the Zephyr Settings API
#define CHIP_DEVICE_CONFIG_SETTINGS_KEY "mt"
#endif // CHIP_DEVICE_CONFIG_SETTINGS_KEY

#ifndef CHIP_DEVICE_CONFIG_OTA_REQUESTOR_REBOOT_DELAY_MS
/// Delay between completing a firmware update download and reboot to apply the update
#define CHIP_DEVICE_CONFIG_OTA_REQUESTOR_REBOOT_DELAY_MS 1000
#endif // CHIP_DEVICE_CONFIG_OTA_REQUESTOR_REBOOT_DELAY_MS

#ifndef CHIP_DEVICE_CONFIG_SERVER_SHUTDOWN_ACTIONS_SLEEP_MS
/// Time to sleep after running server shutdown actions to let lower layers complete the actions.
/// This may include transmitting packets created by the actions.
#define CHIP_DEVICE_CONFIG_SERVER_SHUTDOWN_ACTIONS_SLEEP_MS 500
#endif // CHIP_DEVICE_CONFIG_SERVER_SHUTDOWN_ACTIONS_SLEEP_MS

#ifndef CHIP_DEVICE_CONFIG_HEAP_STATISTICS_MALLINFO
#if !defined(CONFIG_CHIP_MALLOC_SYS_HEAP) && defined(CONFIG_NEWLIB_LIBC)
/// Use mallinfo() to obtain the heap usage statistics exposed by SoftwareDiagnostics cluster attributes.
#define CHIP_DEVICE_CONFIG_HEAP_STATISTICS_MALLINFO 1
#endif // !defined(CONFIG_CHIP_MALLOC_SYS_HEAP) && defined(CONFIG_NEWLIB_LIBC)
#endif // CHIP_DEVICE_CONFIG_HEAP_STATISTICS_MALLINFO

#ifndef CHIP_DEVICE_BLE_ADVERTISING_PRIORITY
/// Priority of the Matter BLE advertising when there are multiple application
/// components that compete for the BLE advertising.
#define CHIP_DEVICE_BLE_ADVERTISING_PRIORITY 0
#endif // CHIP_DEVICE_BLE_ADVERTISING_PRIORITY

// ========== Platform-specific Configuration Overrides =========

#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY
#define CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY (K_PRIO_PREEMPT(1))
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY

#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 8192
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0

#define CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART 0

#ifndef CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART
#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 0
#else
#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART
#endif // CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART

#ifdef CONFIG_CHIP_ENABLE_DNSSD_SRP
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1
#ifdef CONFIG_CHIP_ENABLE_DNS_CLIENT
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_COMMISSIONABLE_DISCOVERY 1
#endif // CONFIG_CHIP_ENABLE_DNS_CLIENT
#endif // CONFIG_CHIP_ENABLE_DNSSD_SRP

#ifdef CONFIG_CHIP_ENABLE_SLEEPY_END_DEVICE_SUPPORT
#define CHIP_DEVICE_CONFIG_ENABLE_SED 1
#define CHIP_DEVICE_CONFIG_THREAD_SSED CONFIG_CHIP_THREAD_SSED
#endif // CONFIG_CHIP_ENABLE_SLEEPY_END_DEVICE_SUPPORT

#ifdef CONFIG_CHIP_COMMISSIONABLE_DEVICE_TYPE
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 1
#endif // CONFIG_CHIP_COMMISSIONABLE_DEVICE_TYPE

#ifdef CONFIG_CHIP_DEVICE_TYPE
#define CHIP_DEVICE_CONFIG_DEVICE_TYPE CONFIG_CHIP_DEVICE_TYPE
#endif // CONFIG_CHIP_DEVICE_TYPE

#ifdef CONFIG_CHIP_EXTENDED_DISCOVERY
#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1
#endif // CONFIG_CHIP_EXTENDED_DISCOVERY
