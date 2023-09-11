/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019-2020 Google LLC.
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines compile-time configuration overrides for the chip Device Layer.
 */

#pragma once

// -------------------- General Configuration --------------------
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 0x00001300

// -------------------- WiFi Station Configuration --------------------

// -------------------- WiFi AP Configuration --------------------
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0

// -------------------- BLE/CHIPoBLE Configuration --------------------
#ifndef CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 1
#endif
#define CHIP_DEVICE_CONFIG_CHIPOBLE_SINGLE_CONNECTION 1
#define CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART 1
#define CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL 60  // 60*0.625ms=37.5ms
#define CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL 600 // 600*0.625ms=375

// -------------------- Time Sync Configuration --------------------

// -------------------- Service Provisioning Configuration --------------------

// -------------------- Just-In-Time Provisioning Configuration --------------------

// -------------------- Service Discovery Configuration -----------------------
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1

// -------------------- Thread Configuration --------------------
#ifndef CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 1
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE 0x00001000
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 1
#endif

// -------------------- Trait Manager Configuration --------------------

// -------------------- Network Telemetry Configuration --------------------

// -------------------- Event Logging Configuration --------------------

// -------------------- Software Update Manager Configuration --------------------
