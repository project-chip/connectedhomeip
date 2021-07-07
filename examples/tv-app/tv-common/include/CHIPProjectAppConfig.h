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
#include <CHIPProjectConfig.h>

// TVs need to be commissioners and likely want to be discoverable
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY 1

// TVs need to be both commissioners and commissionees
#define CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE 1

// TVs do not typically need this - enable for debugging
// #define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT 1

#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 1

#define CHIP_DEVICE_CONFIG_DEVICE_TYPE 35 // 0x0023 = 35 = Video Player

#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_NAME 1

#define CHIP_DEVICE_CONFIG_DEVICE_NAME "Test TV"
