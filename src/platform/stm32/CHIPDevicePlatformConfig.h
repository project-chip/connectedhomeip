/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Platform-specific configuration overrides for CHIP on
 *          the STM32 platform.
 */

#pragma once

// ==================== General Platform Adaptations ====================

#define CHIP_CONFIG_ABORT() abort()

#define CHIP_CONFIG_ENABLE_TUNNELING 0
#define CHIP_CONFIG_MAX_TUNNELS 0
#define CHIP_CONFIG_ENABLE_SERVICE_DIRECTORY 0

#define CHIP_CONFIG_PERSISTED_STORAGE_KEY_TYPE uint16_t
#define CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID 1
#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 2

#define CHIP_CONFIG_LIFETIIME_PERSISTED_COUNTER_KEY 0x01

#if CHIP_ENABLE_OPENTHREAD
#define CHIP_DEVICE_CONFIG_THREAD_FTD 0 // 0 = MTD
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_COMMISSIONABLE_DISCOVERY 1
#endif

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to the platform.
// These can be overridden by the application as needed.

// ...

// ========== Platform-specific Configuration Overrides =========

#define CHIP_DEVICE_CONFIG_CHIP_TASK_NAME "STM32WB TASK"
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE (8 * 1024)

/**
 * CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY
 *
 * The priority of the chip task.
 */
#define CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY 24
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE (4 * 1024)

/**
 * CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY
 *
 * The priority of the OpenThread task.
 */
#define CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY 24

#define CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE 25

/**
 * CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE
 *
 * Reduce packet buffer pool size to 8 (default 15) to reduce ram consumption
 */
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE 8

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0

// Per 5.2.5.2. Commencement Section of CHIP spec, BLE advertisement is
// disabled for Locks and Barrier Access Devices.
#define CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART 0
#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 0

/**
 * CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME
 *
 * The amount of time in miliseconds after which BLE advertisement should be switched from the fast
 * advertising to the slow advertising, counting from the moment of advertisement commencement.
 *
 * Defaults to 30000 (30 seconds).
 */
#ifndef CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME
#define CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME 30000
#endif
// ========== Platform-specific Cluster Overrides =========
#define CHIP_CLUSTER_NETWORK_COMMISSIONING_MAX_NETWORKS 1

// MATTER_DM_OTA_SOFTWARE_UPDATE_PROVIDER_CLUSTER_SERVER_ENDPOINT_COUNT is only defined if the
// cluster is actually enabled in the ZAP config.  To allow operation in setups
#define MATTER_DM_OTA_SOFTWARE_UPDATE_PROVIDER_CLUSTER_SERVER_ENDPOINT_COUNT 1

/**
 * CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC
 *
 * Enables synchronizing the device's real time clock with a remote Chip Time service
 * using the Chip Time Sync protocol.
 */
#define CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC 0

#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 1

/**
 *  @def CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
 *
 *  @brief
 *    Active retransmit interval, or time to wait before retransmission after
 *    subsequent failures in milliseconds.
 *
 *  This is the default value, that might be adjusted by end device depending on its
 *  needs (e.g. sleeping period) using Service Discovery TXT record CRA key.
 *
 */
#define CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL (2000_ms32)

#define OPENTHREAD_CONFIG_IP6_SLAAC_ENABLE 1

/**
 * CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
 *
 * Enable support for Chip-over-BLE (CHIPoBLE).
 */
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 1

#define CHIP_CONFIG_ENABLE_SESSION_RESUMPTION 1

#define CHIP_CONFIG_MEMORY_MGMT_PLATFORM 1
#define CHIP_CONFIG_MEMORY_MGMT_MALLOC 0
