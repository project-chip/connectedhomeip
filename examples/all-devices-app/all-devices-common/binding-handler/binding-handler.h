/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <lib/core/DataModelTypes.h>

namespace chip::app {

// Bit-packing Constants for uintptr_t context
constexpr uintptr_t kBindingCommandMask               = 0xFF;
constexpr uintptr_t kBindingOnOffOnCommandId          = 0x01;
constexpr uintptr_t kBindingOnOffOffCommandId         = 0x02;
constexpr uintptr_t kBindingOnOffToggleCommandId      = 0x03;
constexpr uintptr_t kBindingLevelMoveToLevelCommandId = 0x04;
constexpr uintptr_t kBindingLevelMoveCommandId        = 0x05;
constexpr uintptr_t kBindingLevelStepCommandId        = 0x06;
constexpr uintptr_t kBindingLevelStopCommandId        = 0x07;

/**
 * Register binding callbacks (BoundDeviceChangedHandler and BoundDeviceContextReleaseHandler)
 * with the Binding::Manager singleton.
 */
CHIP_ERROR InitBindingHandler();

// Simulation functions to act on bindings
void SimulateBindingOnOff(EndpointId endpointId, bool on);
void SimulateBindingToggle(EndpointId endpointId);
void SimulateBindingMoveToLevel(EndpointId endpointId, uint8_t newLevel, uint8_t transitionTimeSec, uint8_t optionsMask);
void SimulateBindingMove(EndpointId endpointId, uint8_t moveMode, uint8_t rate, uint8_t optionsMask);
void SimulateBindingStep(EndpointId endpointId, uint8_t stepMode, uint8_t stepSize, uint8_t transitionTimeSec, uint8_t optionsMask);
void SimulateBindingStop(EndpointId endpointId);

} // namespace chip::app
