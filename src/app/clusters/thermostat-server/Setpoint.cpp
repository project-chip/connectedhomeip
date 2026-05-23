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

#include "Setpoint.h"

using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

SystemModeEnum BaseSetpoint::Mode() const
{
    switch (mAttributeId)
    {
    case AbsMinHeatSetpointLimit::Id:
    case AbsMaxHeatSetpointLimit::Id:
    case MinHeatSetpointLimit::Id:
    case MaxHeatSetpointLimit::Id:
    case OccupiedHeatingSetpoint::Id:
    case UnoccupiedHeatingSetpoint::Id:
        return SystemModeEnum::kHeat;
    case AbsMinCoolSetpointLimit::Id:
    case AbsMaxCoolSetpointLimit::Id:
    case MinCoolSetpointLimit::Id:
    case MaxCoolSetpointLimit::Id:
    case OccupiedCoolingSetpoint::Id:
    case UnoccupiedCoolingSetpoint::Id:
        return SystemModeEnum::kCool;
    default:
        return SystemModeEnum::kOff;
    }
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
