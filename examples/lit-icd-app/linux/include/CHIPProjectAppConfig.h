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

// Allows app options (ports) to be configured on launch of app
#define CHIP_DEVICE_ENABLE_PORT_PARAMS 1

// Enable subscriptions synchronization
#define CHIP_CONFIG_SYNCHRONOUS_REPORTS_ENABLED 1

// ICD configurations
#define CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC 3600
#define CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS 10000
#define CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS 5000
