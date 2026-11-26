/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/types/heat-pump/impl/SimulatedHeatPump.h>
#include <oob-accessors/OOBAccessorRegistry.h>

namespace chip::app {

/**
 * Registers the OOB action that makes @p device send a Thermostat SetpointRaiseLower command to the
 * thermostats bound to its endpoint. The action takes a TLV anonymous structure:
 *
 * | Action                     | Tag 1                | Tag 2                                     | Tag 3                     |
 * |----------------------------|----------------------|-------------------------------------------|---------------------------|
 * | HeatPumpSetpointRaiseLower | EndpointId (uint16)  | Mode (uint8, SetpointRaiseLowerModeEnum)  | Amount (int8, 0.1 °C)     |
 *
 * Actions for other endpoints are not handled. An unknown Mode returns ConstraintError. On a heat pump
 * without a Thermostat client the action returns CHIP_ERROR_INVALID_ARGUMENT.
 */
void RegisterOOBAccessors(SimulatedHeatPump & device, OOBAccessorRegistry & registry);

} // namespace chip::app
