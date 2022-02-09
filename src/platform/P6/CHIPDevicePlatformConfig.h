/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *          Platform-specific configuration overrides for the Chip Device Layer
 *          on the PSoC6.
 */

#pragma once

// ==================== Platform Adaptations ====================

#define CHIP_DEVICE_CONFIG_LWIP_WIFI_STATION_IF_NAME "infineon"

#define CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH 0
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0x04B4
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0xF155
#define CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX "INF"
#define CHIP_DEVICE_CONFIG_WIFI_AP_PASSWORD "AP_PASSWORD"
#define CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL 11
#define CHIP_DEVICE_CONFIG_WIFI_AP_SECURITY CY_WCM_SECURITY_WPA2_AES_PSK
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 8192
#define CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY 1
#define CHIP_DEVICE_CONFIG_DEFAULT_STA_SSID ""
#define CHIP_DEVICE_CONFIG_DEFAULT_STA_PASSWORD ""
#define CHIP_DEVICE_CONFIG_DEFAULT_STA_SECURITY CY_WCM_SECURITY_WPA2_AES_PSK
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT 1000
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT                                                                                   \
    "{ 0x53, 0x50, 0x41, 0x4B, 0x45, 0x32, 0x50, 0x20, 0x4B, 0x65, 0x79, 0x20, 0x53, 0x61, 0x6C, 0x74 }"
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER                                                                               \
    "{ 0xab, 0xa6, 0x0c, 0x30, 0x41, 0x6b, 0x8f, 0x41, 0x77, 0xf5, 0xe1, 0x6a, 0xd5, 0x14, 0xcf, 0xd9, 0x57, 0x75, 0x13, 0xf0, 0x2f,  \
        0xd6, 0x05, 0x06, 0xb1, 0x04, 0x9d, 0x0f, 0x2c, 0x73, 0x10, 0x01, 0x0e, 0x5e, 0x40, 0xbf, 0xd8, 0x6b, 0x4e, 0xf6, 0x81,    \
        0xa8, 0x8b, 0x71, 0xe9, 0xe2, 0xa8, 0x53, 0x98, 0x5a, 0x7d, 0xef, 0x91, 0x6e, 0xa3, 0x0e, 0x01, 0xb8, 0x72, 0x2f, 0xbf,    \
        0x7d, 0x0e, 0x38, 0x85, 0x6c, 0x12, 0xcd, 0x64, 0xc2, 0x25, 0xbb, 0x24, 0xef, 0x21, 0x41, 0x7e, 0x0e, 0x44, 0xe5 }"
