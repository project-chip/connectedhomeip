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
 *          on Qorvo QPG platforms.
 */

#pragma once

// ==================== Platform Adaptations ====================

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0

#ifndef CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 1
#endif

#if CHIP_ENABLE_OPENTHREAD
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_COMMISSIONABLE_DISCOVERY 1
#define OPENTHREAD_CONFIG_IP6_SLAAC_ENABLE 1
#endif

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to the platform.
// These can be overridden by the application as needed.

// ...

// ========== Platform-specific Configuration Overrides =========

#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE (8 * 1024)
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE

#ifndef CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE (7 * 1024)
#endif // CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE

#define CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE 25

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0

