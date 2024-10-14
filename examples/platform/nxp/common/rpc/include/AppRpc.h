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

#pragma once

#include <lib/core/CHIPError.h>

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
#include "pigweed/rpc_services/Button.h"
#endif

namespace chip::NXP::App::Rpc {

/**
 * @brief Initialization of RPC-related board code.
 *
 * For example, enabling clock for UART interface
 * and initializing board pins.
 *
 */
CHIP_ERROR Init();

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
/**
 * @brief Rebooting the device.
 *
 * RPC implementation will delegate reboot procedure to this function.
 *
 */
void Reboot();
#endif

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
/**
 * @brief Handles button events.
 *
 * RPC implementation will delegate handling events to this function.
 */
void ButtonHandler(const chip_rpc_ButtonEvent & request);
#endif

} // namespace chip::NXP::App::Rpc
