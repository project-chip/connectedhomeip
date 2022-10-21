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
 *          for the Texas Instruments CC32XX platform.
 *
 * NOTE: currently a bare-bones implementation to allow for building.
 */

#ifndef CHIP_DEVICE_PLATFORM_CONFIG_H
#define CHIP_DEVICE_PLATFORM_CONFIG_H

// XXX: Seth this needs to be updated for TI devices

// ==================== Platform Adaptations ====================

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 1
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 1
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP | CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

// ========== Platform-specific Configuration =========

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0
#define CHIP_DEVICE_CONFIG_ENABLE_TUNNEL_TELEMETRY 0

// ========== CHIP Platform Configuration =========
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE (8192)
#endif // CHIP_DEVICE_PLATFORM_CONFIG_H
