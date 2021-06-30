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
 *          on nRF Connect SDK platform.
 */

#pragma once

#include "autoconf.h"

// ==================== Platform Adaptations ====================

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0

#define CHIP_DEVICE_CONFIG_ENABLE_THREAD CONFIG_NET_L2_OPENTHREAD
#define CHIP_DEVICE_CONFIG_THREAD_FTD CONFIG_OPENTHREAD_FTD

#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE CONFIG_BT

#define CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC 0

#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_CRIT_EIDC_KEY 2
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_PROD_EIDC_KEY 3
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_INFO_EIDC_KEY 4
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_DEBUG_EIDC_KEY 5

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to Zephyr platforms.
// These can be overridden by the application as needed.

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
#define CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH 0

#define CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED CONFIG_CHIP_DISABLE_CHIPOBLE_ADVERTISING_WHEN_PROVISIONED

#define CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART 0

#ifndef CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART
#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 0
#else
#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART
#endif // CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART

#ifdef CONFIG_CHIP_ENABLE_DNSSD_SRP
#define CHIP_DEVICE_CONFIG_ENABLE_MDNS 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1
#endif // CONFIG_CHIP_ENABLE_DNSSD_SRP
