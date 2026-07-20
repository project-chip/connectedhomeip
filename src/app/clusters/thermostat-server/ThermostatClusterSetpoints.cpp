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

Status ValidateSetpointChange(Setpoints & setpoints, chip::AttributeId attributeId, temperature value,
                              SetpointAttributes & changedAttributes)
{
    switch (attributeId)
    {
    case OccupiedHeatingSetpoint::Id:
        return setpoints.ChangeRangeHeating(setpoints.occupiedRange, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case OccupiedCoolingSetpoint::Id:
        return setpoints.ChangeRangeCooling(setpoints.occupiedRange, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case UnoccupiedHeatingSetpoint::Id:
        if (!setpoints.occupancySupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeRangeHeating(setpoints.unoccupiedRange, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case UnoccupiedCoolingSetpoint::Id:
        if (!setpoints.occupancySupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeRangeCooling(setpoints.unoccupiedRange, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case MinHeatSetpointLimit::Id:
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeLimitMinimum(setpoints.userHeatLimits, setpoints.absoluteHeatLimits, value, changedAttributes);
    case MaxHeatSetpointLimit::Id:
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeLimitMaximum(setpoints.userHeatLimits, setpoints.absoluteHeatLimits, value, changedAttributes);
    case MinCoolSetpointLimit::Id:
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeLimitMinimum(setpoints.userCoolLimits, setpoints.absoluteCoolLimits, value, changedAttributes);
    case MaxCoolSetpointLimit::Id:
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeLimitMaximum(setpoints.userCoolLimits, setpoints.absoluteCoolLimits, value, changedAttributes);
    default:
        return Status::UnsupportedAttribute;
    }
}

void HandleSetpointWrite(const ConcreteAttributePath & attributePath)
{
    Setpoints setpoints;
    auto status = LoadSetpoints(attributePath.mEndpointId, setpoints);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Thermostat: HandleSetpointWrite: failed to load setpoints: " ChipLogFormatIMStatus,
                     ChipLogValueIMStatus(status));
        return;
    }
    temperature temp;
    switch (attributePath.mAttributeId)
    {
    case OccupiedHeatingSetpoint::Id:
        temp = setpoints.occupiedRange.heating.Temperature();
        break;
    case OccupiedCoolingSetpoint::Id:
        temp = setpoints.occupiedRange.cooling.Temperature();
        break;
    case UnoccupiedHeatingSetpoint::Id:
        temp = setpoints.unoccupiedRange.heating.Temperature();
        break;
    case UnoccupiedCoolingSetpoint::Id:
        temp = setpoints.unoccupiedRange.cooling.Temperature();
        break;
    case MinHeatSetpointLimit::Id:
        temp = setpoints.userHeatLimits.minimum.Temperature();
        break;
    case MaxHeatSetpointLimit::Id:
        temp = setpoints.userHeatLimits.maximum.Temperature();
        break;
    case MinCoolSetpointLimit::Id:
        temp = setpoints.userCoolLimits.minimum.Temperature();
        break;
    case MaxCoolSetpointLimit::Id:
        temp = setpoints.userCoolLimits.maximum.Temperature();
        break;
    default:
        ChipLogError(Zcl, "Thermostat: HandleSetpointWrite: unexpected attributeId: 0x%" PRIx32, attributePath.mAttributeId);
        return;
    }

    SetpointAttributes changedAttributes;
    changedAttributes.Set(attributePath.mAttributeId);
    changedAttributes.ClearFirstDirtyAttribute();

    // In theory, this should always succeed, as the values will have been filtered in
    // MatterThermostatClusterServerPreAttributeChangedCallback
    if (ValidateSetpointChange(setpoints, attributePath.mAttributeId, temp, changedAttributes) != Status::Success)
    {
        ChipLogError(Zcl, "Thermostat: HandleSetpointWrite: failed to validate setpoint after write");
        return;
    }

    // Clear the attribute that was just written
    changedAttributes.Clear(attributePath.mAttributeId);
    if (!changedAttributes.Empty())
    {
        // If there were any other attributes that were changed, we save the first one
        status = SaveFirstDirtySetpoint(attributePath.mEndpointId, setpoints, changedAttributes);
        if (status != Status::Success)
        {
            ChipLogError(Zcl, "Thermostat: HandleSetpointWrite: failed to save dirty setpoint: " ChipLogFormatIMStatus,
                         ChipLogValueIMStatus(status));
        }
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
        isOccupied = occupancy.Has(OccupancyBitmap::kOccupied) ? OccupancyBitmap::kOccupied : OccupancyBitmap(0);
    }
    else
    {
        isOccupied = OccupancyBitmap::kOccupied;
    }

    auto & range = setpoints.GetRange(isOccupied);

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

    SetpointAttributes changedAttributes;
    status = setpoints.ChangeRange(range, heat, cool, Setpoints::ClampMode::kClamp, changedAttributes);
    if (status != Status::Success)
    {
        return status;
    }
    return SaveSetpoints(endpointId, setpoints, changedAttributes);
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
