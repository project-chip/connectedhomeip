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

// include the CHIPProjectConfig from config/standalone

#ifndef CHIP_CONFIG_KVS_PATH
#define CHIP_CONFIG_KVS_PATH "/tmp/chip_casting_kvs"
#endif

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY 0

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT 1

#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 0

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 1

#define CHIP_DEVICE_CONFIG_DEVICE_TYPE 41 // 0x0029 = 41 = Video Client

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_NAME 1

#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021

#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00

#define CHIP_DEVICE_CONFIG_DEVICE_NAME "Test TV casting app"

#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 0

// For casting, we need to allow more ACL entries, and more complex entries
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_TARGETS_PER_ENTRY 20
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_SUBJECTS_PER_ENTRY 20
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_ENTRIES_PER_FABRIC 20

// Enable some test-only interaction model APIs.
#define CONFIG_BUILD_FOR_HOST_UNIT_TEST 1

#define CHIP_ENABLE_ROTATING_DEVICE_ID 1

#define CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID_LENGTH 128

// Disable this since it should not be enabled for production setups
#define CHIP_DEVICE_CONFIG_ENABLE_TEST_SETUP_PARAMS 0

#define CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT 4

// cached players that were seen before this window (in hours) will not be surfaced as "discovered"
#define CHIP_DEVICE_CONFIG_STR_CACHE_LAST_DISCOVERED_HOURS 7 * 24

// time (in sec) assumed to be required for player to wake up after sending WoL magic packet
#define CHIP_DEVICE_CONFIG_STR_WAKE_UP_DELAY_SEC 10

// delay (in sec) before which we assume undiscovered cached players may be in STR mode
#define CHIP_DEVICE_CONFIG_STR_DISCOVERY_DELAY_SEC 5

// Include the CHIPProjectConfig from config/standalone
// Add this at the end so that we can hit our #defines first
#include <CHIPProjectConfig.h>
