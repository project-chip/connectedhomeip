/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

// ==================== Platform Adaptations ====================

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0

// ========== Platform-specific Configuration =========

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0
#define CHIP_DEVICE_CONFIG_ENABLE_TUNNEL_TELEMETRY 0

// ========== CHIP Platform Configuration =========
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE (4 * 1024)
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE (6 * 1024)
#define CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE 15

#if CHIP_ENABLE_OPENTHREAD
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1
#endif

// Per 5.2.5.2. Commencement Section of CHIP spec, BLE advertisement is
// disabled for Locks and Barrier Access Devices.
#define CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART 0
#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 1

#ifndef CHIP_DEVICE_CONFIG_ENABLE_DNSSD
#define CHIP_DEVICE_CONFIG_ENABLE_DNSSD 1
#endif
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DISCOVERY 1

#define CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_GLOBAL_EIDC_KEY 2
