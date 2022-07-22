/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Example project configuration file for CHIP.
 *
 *          This is a place to put application or project-specific overrides
 *          to the default configuration values for general CHIP features.
 *
 */

#pragma once

// TVs need to be commissioners and likely want to be discoverable
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY 1

// TVs need to be both commissioners and commissionees
#define CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE 1

// TVs that are not commissionees,
// or that don't automatically enter commissioning mode should set this to 0
#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 1

// TVs do not typically need this - enable for debugging
// #define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT 1

// Enable extended discovery, set timeout to 24 hours
#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1
#define CHIP_DEVICE_CONFIG_EXTENDED_DISCOVERY_TIMEOUT_SECS (24 * 60 * 60)

// Advertise TV device type in DNS-SD
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 1

#define CHIP_DEVICE_CONFIG_DEVICE_TYPE 35 // 0x0023 = 35 = Video Player

// Include device name in discovery for casting use case
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_NAME 1
#define CHIP_DEVICE_CONFIG_DEVICE_NAME "Test TV"

// Enable app platform
#define CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED 1

// overrides CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT in CHIPProjectConfig
#define CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT 16

// For casting, we need to allow more ACL entries, and more complex entries
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_TARGETS_PER_ENTRY 20
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_SUBJECTS_PER_ENTRY 20
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_ENTRIES_PER_FABRIC 20

// Change port to make it easy to run against tv-casting-app
#define CHIP_PORT 5640

// Create a dedicated file for storage to make it easy to run against other apps
// #define CHIP_CONFIG_KVS_PATH "/tmp/chip_tv_kvs"

// include the CHIPProjectConfig from config/standalone
#include <CHIPProjectConfig.h>
