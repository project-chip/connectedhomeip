/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
