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

#include <CHIPProjectConfig.h>

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY 0

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT 1

#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 1

#define CHIP_DEVICE_CONFIG_DEVICE_TYPE 35 // 0x0023 = 35 = Video Player

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_NAME 1

#define CHIP_DEVICE_CONFIG_DEVICE_NAME "Test TV casting app"

#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 0

// Enable some test-only interaction model APIs.
#define CONFIG_BUILD_FOR_HOST_UNIT_TEST 1
