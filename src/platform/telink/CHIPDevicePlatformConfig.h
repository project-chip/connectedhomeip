/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *          on Telink platform.
 */

#pragma once

#include "autoconf.h"

// ==================== Platform Adaptations ====================

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0

#define CHIP_DEVICE_CONFIG_ENABLE_THREAD CONFIG_NET_L2_OPENTHREAD

#define CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC 0

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to Zephyr platforms.
// These can be overridden by the application as needed.

#ifndef CHIP_DEVICE_CONFIG_SETTINGS_KEY
/// Key for all Matter persistent data stored using the Zephyr Settings API
#define CHIP_DEVICE_CONFIG_SETTINGS_KEY "mt"
#endif // CHIP_DEVICE_CONFIG_SETTINGS_KEY

#ifndef CHIP_DEVICE_CONFIG_HEAP_STATISTICS_MALLINFO
#if !defined(CONFIG_CHIP_MALLOC_SYS_HEAP) && defined(CONFIG_NEWLIB_LIBC)
/// Use mallinfo() to obtain the heap usage statistics exposed by SoftwareDiagnostics cluster attributes.
#define CHIP_DEVICE_CONFIG_HEAP_STATISTICS_MALLINFO 1
#endif // !defined(CONFIG_CHIP_MALLOC_SYS_HEAP) && defined(CONFIG_NEWLIB_LIBC)
#endif // CHIP_DEVICE_CONFIG_HEAP_STATISTICS_MALLINFO

// ========== Platform-specific Configuration Overrides =========

#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY
#define CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY (K_PRIO_COOP(CONFIG_NUM_COOP_PRIORITIES - 1))
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY

#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 8192
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0
#define CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH 0

#define CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART 0

#ifndef CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART
#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 0
#else
#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART
#endif // CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART

#ifdef CONFIG_CHIP_ENABLE_DNSSD_SRP
#define CHIP_DEVICE_CONFIG_ENABLE_MDNS 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DISCOVERY 1
#ifdef CONFIG_CHIP_ENABLE_DNS_CLIENT
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_COMMISSIONABLE_DISCOVERY 1
#endif // CONFIG_CHIP_ENABLE_DNS_CLIENT
#endif // CONFIG_CHIP_ENABLE_DNSSD_SRP
