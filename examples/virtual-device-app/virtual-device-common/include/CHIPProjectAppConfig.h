/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY 0

#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 1

// Enable extended discovery, set timeout to 24 hours
#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 0
#define CHIP_DEVICE_CONFIG_EXTENDED_DISCOVERY_TIMEOUT_SECS (24 * 60 * 60)

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 1

#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0xFFF1
#define CHIP_DEVICE_CONFIG_DEVICE_TYPE 259 // 0x0103 On/Off Light Switch

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_NAME 1
#define CHIP_DEVICE_CONFIG_DEVICE_NAME "Matter Virtual Device"

// Enable app platform
#define CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED 0

// overrides CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT in CHIPProjectConfig
#define CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT 16

// include the CHIPProjectConfig from config/standalone
#include <CHIPProjectConfig.h>
