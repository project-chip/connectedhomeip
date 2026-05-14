/*
 *
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

#include <app/ConcreteAttributePath.h>
#include <app/util/attribute-storage.h>
#include <protocols/interaction_model/Constants.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

using temperature = int16_t;

constexpr temperature kDefaultAbsMinHeatSetpointLimit = 700;  // 7C (44.5 F) is the default
constexpr temperature kDefaultAbsMaxHeatSetpointLimit = 3000; // 30C (86 F) is the default
constexpr temperature kDefaultAbsMinCoolSetpointLimit = 1600; // 16C (61 F) is the default
constexpr temperature kDefaultAbsMaxCoolSetpointLimit = 3200; // 32C (90 F) is the default
constexpr temperature kDefaultDeadBand        = 200; // 2.0C is the default; this changed from 2.5C in revision 8 of this cluster
constexpr temperature kDefaultHeatingSetpoint = 2000;
constexpr temperature kDefaultCoolingSetpoint = 2600;

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
