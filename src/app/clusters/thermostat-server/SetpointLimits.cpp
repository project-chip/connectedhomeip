/**
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <clusters/Thermostat/Metadata.h>

#include "SetpointLimits.h"
#include "Setpoints.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

bool UserSetpointLimits::IsValid() const
{
    if (!absoluteLimits.IsValid())
    {
        return false;
    }
    if (minimum.HasTemperature() && !absoluteLimits.Valid(minimum.Temperature()))
    {
        return false;
    }
    if (maximum.HasTemperature() && !absoluteLimits.Valid(maximum.Temperature()))
    {
        return false;
    }
    return SetpointLimits::IsValid();
}

temperature UserSetpointLimits::Minimum() const
{
    return minimum.HasTemperature() ? minimum.Temperature() : absoluteLimits.Minimum();
}
temperature UserSetpointLimits::Maximum() const
{
    return maximum.HasTemperature() ? maximum.Temperature() : absoluteLimits.Maximum();
}

/*EffectiveSetpointLimits::EffectiveSetpointLimits(const Setpoints & sp, SystemModeEnum systemMode) :
    SetpointLimits(systemMode), setpoints(sp)
{
    temperature minHeat = setpoints.userHeatLimits.Minimum();
    temperature maxHeat = setpoints.userHeatLimits.Maximum();
    temperature minCool = setpoints.userCoolLimits.Minimum();
    temperature maxCool = setpoints.userCoolLimits.Maximum();

    switch (systemMode)
    {
    case SystemModeEnum::kHeat:
        minimum = minHeat;
        maximum = setpoints.autoSupported ? std::min(maxHeat, static_cast<int16_t>(maxCool - setpoints.deadBand)) : maxHeat;
        break;
    default:
        minimum = setpoints.autoSupported ? std::max(minCool, static_cast<int16_t>(minHeat + setpoints.deadBand)) : minCool;
        maximum = maxCool;
        break;
    }
}*/

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
