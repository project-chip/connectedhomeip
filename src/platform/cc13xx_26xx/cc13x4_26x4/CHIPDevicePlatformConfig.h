/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the chip Device Layer
 *          for the Texas Instruments CC13XX_CC26XX platform.
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

#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE (8192)

#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE (7168)

#define BLEMANAGER_EVENT_HANDLER_STACK_SIZE (924)
#define BLEMANAGER_EVENT_HANDLER_PRIORITY (2)

#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 1

// Per 5.2.5.2. Commencement Section of CHIP spec, BLE advertisement is
// disabled for Locks and Barrier Access Devices.
#define CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART 0
#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 0

#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 1
