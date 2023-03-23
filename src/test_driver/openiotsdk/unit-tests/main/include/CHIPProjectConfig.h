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

// Enable support functions for parsing command-line arguments
#define CHIP_CONFIG_ENABLE_ARG_PARSER 1
#define CHIP_CONFIG_NON_POSIX_LONG_OPT 1

#define CHIP_SYSTEM_CONFIG_NUM_TIMERS 32
#define CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE 1

#define CONFIG_BUILD_FOR_HOST_UNIT_TEST 1

#define CONFIG_IM_BUILD_FOR_UNIT_TEST 1
#define CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT 64
#define CHIP_CONFIG_MAX_FABRICS 16
#define CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE 500

#define CHIP_SYSTEM_CONFIG_POOL_USE_HEAP 1
