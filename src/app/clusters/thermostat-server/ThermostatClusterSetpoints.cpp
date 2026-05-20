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
#include "Setpoint.h"
#include "Temperature.h"
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

Status HandleSetpointChange(Setpoints & setpoints, chip::AttributeId attributeId, temperature value,
                            SetpointAttributes & changedAttributes)
{
    switch (attributeId)
    {
    case OccupiedHeatingSetpoint::Id:
        return setpoints.ChangeRangeHeating(setpoints.occupied, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case OccupiedCoolingSetpoint::Id:
        return setpoints.ChangeRangeCooling(setpoints.occupied, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case UnoccupiedHeatingSetpoint::Id:
        if (!setpoints.occupancySupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeRangeHeating(setpoints.unoccupied, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case UnoccupiedCoolingSetpoint::Id:
        if (!setpoints.occupancySupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeRangeCooling(setpoints.unoccupied, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case MinHeatSetpointLimit::Id:
        if (!setpoints.heatSupported)
            return Status::UnsupportedAttribute;
        return setpoints.ChangeLimitMinimum(setpoints.userHeatLimits, value, changedAttributes);
    case MaxHeatSetpointLimit::Id:
        if (!setpoints.heatSupported)
            return Status::UnsupportedAttribute;
        return setpoints.ChangeLimitMaximum(setpoints.userHeatLimits, value, changedAttributes);
    case MinCoolSetpointLimit::Id:
        if (!setpoints.coolSupported)
            return Status::UnsupportedAttribute;
        return setpoints.ChangeLimitMinimum(setpoints.userCoolLimits, value, changedAttributes);
    case MaxCoolSetpointLimit::Id:
        if (!setpoints.coolSupported)
            return Status::UnsupportedAttribute;
        return setpoints.ChangeLimitMaximum(setpoints.userCoolLimits, value, changedAttributes);
    case MinSetpointDeadBand::Id:
        return Status::Success;
    default:
        return Status::UnsupportedAttribute;
    }
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

    chip::Optional<temperature> heat;
    chip::Optional<temperature> cool;

    switch (mode)
    {
    case SetpointRaiseLowerModeEnum::kBoth:
        if (setpoints.heatSupported)
        {
            heat.SetValue(static_cast<temperature>(range.heating.Temperature() + amount));
        }
        if (setpoints.coolSupported)
        {
            cool.SetValue(static_cast<temperature>(range.cooling.Temperature() + amount));
        }
        break;
    case SetpointRaiseLowerModeEnum::kHeat:
        if (setpoints.heatSupported)
        {
            heat.SetValue(static_cast<temperature>(range.heating.Temperature() + amount));
        }
        break;
    case SetpointRaiseLowerModeEnum::kCool:
        if (setpoints.coolSupported)
        {
            cool.SetValue(static_cast<temperature>(range.cooling.Temperature() + amount));
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
        SetpointAttributes changedAttributes;
        status = setpoints.ChangeRange(range, heat, cool, Setpoints::ClampMode::kClamp, changedAttributes);
        if (status == Status::Success)
        {
            return SaveSetpoints(endpointId, setpoints, changedAttributes);
        }
    }

    return Status::Success;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
