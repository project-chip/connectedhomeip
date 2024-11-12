/*
 *    Copyright (c) 2024 Project CHIP Authors
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

// FabricSync needs to be commissioners and likely want to be discoverable
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY 1

// FabricSync will often enable this feature
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_PASSCODE 1

// FabricSync needs to be both commissioners and commissionees
#define CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE 1

// See issue 23625.
#define CHIP_CONFIG_UNSAFE_SUBSCRIPTION_EXCHANGE_MANAGER_USE 1

// Enable app platform
#define CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED 1

// overrides CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT in CHIPProjectConfig
#define CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT 16

// Allows app options (ports) to be configured on launch of app
#define CHIP_DEVICE_ENABLE_PORT_PARAMS 1

// Include the CHIPProjectConfig from platform implementation config
#include <CHIPProjectConfig.h>
