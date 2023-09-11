/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2020 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the chip Device Layer
 *          on MW320 platforms using the NXP SDK.
 */

#pragma once

// ==================== Platform Adaptations ====================

#define MW320_NO_ERRORS 0
#define MW320_ENTRY_NOT_FOUND 1

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 1
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0

#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 0

#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_CRIT_EIDC_KEY 2
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_PROD_EIDC_KEY 3
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_INFO_EIDC_KEY 4
#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_DEBUG_EIDC_KEY 5

#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_GLOBAL_EIDC_KEY 2

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to the MW320 platform.
// These can be overridden by the application as needed.

// ========== Platform-specific Configuration Overrides =========
#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 9216
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE

/*#ifndef CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE 8192
#endif // CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE
*/
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0

// mdns/mdns_sd ++
//#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DISCOVERY 1
#define CHIP_DEVICE_CONFIG_ENABLE_DNSSD 1
// mdns/dns_sd --
