/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "ThermostatClusterSetpoints.h"
#include "ThermostatCluster.h"

#include "Setpoints.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <clusters/Thermostat/Metadata.h>

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

Status HandleSetpointChange(Setpoints & setpoints, const AttributeId attributeId, int16_t temperature,
                            chip::BitFlags<SetpointAttributes> & affectedAttributes)
{
    Status status = Status::Success;
    switch (attributeId)
    {
    case OccupiedHeatingSetpoint::Id: {
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        auto range = setpoints.GetRange(OccupancyBitmap::kOccupied);
        status     = setpoints.ChangeRange(range, chip::Optional(temperature), chip::Optional<int16_t>::Missing(), Setpoints::ClampMode::kDontClamp,
                                           affectedAttributes);
    }
    break;
    case OccupiedCoolingSetpoint::Id: {
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        auto range = setpoints.GetRange(OccupancyBitmap::kOccupied);
        status     = setpoints.ChangeRange(range, chip::Optional<int16_t>::Missing(), chip::Optional(temperature), Setpoints::ClampMode::kDontClamp,
                                           affectedAttributes);
    }
    break;
    case UnoccupiedHeatingSetpoint::Id: {
        if (!setpoints.heatSupported || !setpoints.occupancySupported)
        {
            return Status::UnsupportedAttribute;
        }
        auto range = setpoints.GetRange(OccupancyBitmap(0));
        status     = setpoints.ChangeRange(range, chip::Optional(temperature), chip::Optional<int16_t>::Missing(), Setpoints::ClampMode::kDontClamp,
                                           affectedAttributes);
    }
    break;
    case UnoccupiedCoolingSetpoint::Id: {
        if (!setpoints.coolSupported || !setpoints.occupancySupported)
        {
            return Status::UnsupportedAttribute;
        }
        auto range = setpoints.GetRange(OccupancyBitmap(0));
        status     = setpoints.ChangeRange(range, chip::Optional<int16_t>::Missing(), chip::Optional(temperature), Setpoints::ClampMode::kDontClamp,
                                           affectedAttributes);
    }
    break;
    case MinHeatSetpointLimit::Id: {
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        status = setpoints.ChangeLimits(setpoints.heatLimitsOverride, MakeOptional(temperature), chip::Optional<int16_t>::Missing(),
                               affectedAttributes);
    }
    break;
    case MaxHeatSetpointLimit::Id: {
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        status = setpoints.ChangeLimits(setpoints.heatLimitsOverride, chip::Optional<int16_t>::Missing(), MakeOptional(temperature),
                               affectedAttributes);
    }
    break;
    case MinCoolSetpointLimit::Id: {
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        status = setpoints.ChangeLimits(setpoints.coolLimitsOverride, MakeOptional(temperature), chip::Optional<int16_t>::Missing(),
                               affectedAttributes);
    }
    break;
    case MaxCoolSetpointLimit::Id: {
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        status = setpoints.ChangeLimits(setpoints.coolLimitsOverride, chip::Optional<int16_t>::Missing(), MakeOptional(temperature),
                               affectedAttributes);
    }
    break;
    case MinSetpointDeadBand::Id: {
        status = Status::Success;
    }
    }
    return status;
}

Status SetpointRaiseLower(const EndpointId endpointId, const Commands::SetpointRaiseLower::DecodableType & commandData)
{

    auto & mode    = commandData.mode;
    int16_t amount = static_cast<int16_t>(commandData.amount * 10);

    Setpoints setpoints;
    auto status = LoadSetpoints(endpointId, setpoints);
    if (status != Status::Success)
    {
        return status;
    }

    OccupancyBitmap isOccupied = OccupancyBitmap(0);
    if (setpoints.occupancySupported)
    {
        BitMask<OccupancyBitmap, uint8_t> occupancy;
        status = Occupancy::Get(endpointId, &occupancy);
        if (status != Status::Success)
        {
            return status;
        }
        isOccupied = OccupancyBitmap::kOccupied;
    }
    else
    {
        isOccupied = OccupancyBitmap::kOccupied;
    }

    auto range = setpoints.GetRange(isOccupied);

    chip::Optional<int16_t> heat;
    chip::Optional<int16_t> cool;

    switch (mode)
    {
    case SetpointRaiseLowerModeEnum::kBoth:
        if (setpoints.heatSupported)
        {
            heat.SetValue(static_cast<int16_t>(range.heatingSetpoint + amount));
        }
        if (setpoints.coolSupported)
        {
            cool.SetValue(static_cast<int16_t>(range.coolingSetpoint + amount));
        }
        break;
    case SetpointRaiseLowerModeEnum::kHeat:
        if (setpoints.heatSupported)
        {
            heat.SetValue(static_cast<int16_t>(range.heatingSetpoint + amount));
        }
        break;
    case SetpointRaiseLowerModeEnum::kCool:
        if (setpoints.coolSupported)
        {
            cool.SetValue(static_cast<int16_t>(range.coolingSetpoint + amount));
        }
        break;
    default:
        return Status::InvalidCommand;
    }

    if (!heat.HasValue() && !cool.HasValue())
    {
        return Status::InvalidCommand;
    }
    if (heat.HasValue() || cool.HasValue())
    {
        chip::BitFlags<SetpointAttributes> affectedAttributes;
        status = setpoints.ChangeRange(range, heat, cool, Setpoints::ClampMode::kClamp, affectedAttributes);
        if (status == Status::Success)
        {
            return SaveSetpoints(endpointId, setpoints, affectedAttributes);
        }
    }

    return Status::Success;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
