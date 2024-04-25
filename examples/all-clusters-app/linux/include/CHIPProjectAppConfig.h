/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

// All clusters app has 3 group endpoints. This needs to defined here so that
// CHIP_CONFIG_MAX_GROUPS_PER_FABRIC is properly configured.
#define CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC 3

// Allows app options (ports) to be configured on launch of app
#define CHIP_DEVICE_ENABLE_PORT_PARAMS 1

// Expose the device type in the advertisement for CI testing.
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 1

// Marks that a ModeBase Derived cluster is being used.
#define MATTER_DM_PLUGIN_MODE_BASE

// Enable batching of up to 5 commands.
#define CHIP_CONFIG_MAX_PATHS_PER_INVOKE 5

#define CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER 1
