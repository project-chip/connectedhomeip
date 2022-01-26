/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *          Platform-specific configuration overrides for the chip Device Layer
 *          on Ameba platform.
 */

#pragma once

// ==================== Platform Adaptations ====================
#define CHIP_CONFIG_ERROR_CLASS 1

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0

#if CHIP_ENABLE_OPENTHREAD
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 1
#endif

#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 1

#define CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC 0

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to the platform.
// These can be overridden by the application as needed.

// ...

// ========== Platform-specific Configuration Overrides =========

#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 8192
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE

#ifndef CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE 4096
#endif // CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0
#define CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH 0

#define CHIP_DEVICE_LAYER_NONE 0

// Use a default pairing code if one hasn't been provisioned in flash.
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT 1000
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT                                                                                   \
    "{ 0x53, 0x50, 0x41, 0x4B, 0x45, 0x32, 0x50, 0x20, 0x4B, 0x65, 0x79, 0x20, 0x53, 0x61, 0x6C, 0x74 }"
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER                                                                               \
    "{ 0xab, 0xa6, 0x0c, 0x30, 0x41, 0x6b, 0x8f, 0x41, 0x77, 0xf5, 0xe1, 0x6a, 0xd5, 0x14, 0xcf, 0xd9, 0x57, 0x75, 0x13, 0xf0, 0x2f,  \
        0xd6, 0x05, 0x06, 0xb1, 0x04, 0x9d, 0x0f, 0x2c, 0x73, 0x10, 0x01, 0x0e, 0x5e, 0x40, 0xbf, 0xd8, 0x6b, 0x4e, 0xf6, 0x81,    \
        0xa8, 0x8b, 0x71, 0xe9, 0xe2, 0xa8, 0x53, 0x98, 0x5a, 0x7d, 0xef, 0x91, 0x6e, 0xa3, 0x0e, 0x01, 0xb8, 0x72, 0x2f, 0xbf,    \
        0x7d, 0x0e, 0x38, 0x85, 0x6c, 0x12, 0xcd, 0x64, 0xc2, 0x25, 0xbb, 0x24, 0xef, 0x21, 0x41, 0x7e, 0x0e, 0x44, 0xe5 }"

#define CONFIG_RENDEZVOUS_MODE 6

#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 1
//#define CHIP_DEVICE_CONFIG_UNIQUE_ID "00112233445566778899AABBCCDDEEFF"
