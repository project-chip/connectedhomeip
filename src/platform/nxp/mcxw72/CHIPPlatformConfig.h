/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Google LLC.
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
 *          Platform-specific configuration overrides for CHIP on
 *          NXP MCXW72 platform.
 */

#pragma once

/**
 * @def CHIP_DEVICE_CONFIG_KVS_WEAR_STATS
 *
 * @brief Toggle support for key value store wear stats on or off.
 */
#ifndef CHIP_DEVICE_CONFIG_KVS_WEAR_STATS
#define CHIP_DEVICE_CONFIG_KVS_WEAR_STATS 0
#endif // CHIP_DEVICE_CONFIG_KVS_WEAR_STATS

/**
 * @def CHIP_IM_MAX_NUM_COMMAND_HANDLER
 *
 * @brief Defines the maximum number of CommandHandler, limits the number of active commands transactions on server.
 */
#ifndef CHIP_IM_MAX_NUM_COMMAND_HANDLER
#define CHIP_IM_MAX_NUM_COMMAND_HANDLER 2
#endif // CHIP_IM_MAX_NUM_COMMAND_HANDLER

/**
 * @def CHIP_IM_MAX_NUM_WRITE_HANDLER
 *
 * @brief Defines the maximum number of WriteHandler, limits the number of active write transactions on server.
 */
#ifndef CHIP_IM_MAX_NUM_WRITE_HANDLER
#define CHIP_IM_MAX_NUM_WRITE_HANDLER 2
#endif // CHIP_IM_MAX_NUM_WRITE_HANDLER

// Include default nxp platform configurations
#include "platform/nxp/common/CHIPNXPPlatformDefaultConfig.h"
