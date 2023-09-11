/*
 *
 * SPDX-FileCopyrightText: 2021-2022 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific configuration overrides for the Chip Device Layer
 *          on the PSoC6.
 */

#pragma once

// ==================== Platform Adaptations ====================

#define CHIP_DEVICE_CONFIG_LWIP_WIFI_STATION_IF_NAME "infineon"

/* The VendorName attribute of the Basic cluster. */
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME "Infineon"

/* The VendorID attribute of the Basic cluster. */
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0xFFF1

#define CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX "INF"
#define CHIP_DEVICE_CONFIG_WIFI_AP_PASSWORD "AP_PASSWORD"
#define CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL 11
#define CHIP_DEVICE_CONFIG_WIFI_AP_SECURITY CY_WCM_SECURITY_WPA2_AES_PSK
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 8192
#define CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY 1
#define CHIP_DEVICE_CONFIG_DEFAULT_STA_SSID ""
#define CHIP_DEVICE_CONFIG_DEFAULT_STA_PASSWORD ""
#define CHIP_DEVICE_CONFIG_DEFAULT_STA_SECURITY CY_WCM_SECURITY_UNKNOWN
#define CHIP_DEVICE_CONFIG_ENABLE_TEST_SETUP_PARAMS 1
