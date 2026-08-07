/*
 *
 *    Copyright (c) 2024-2025 Project CHIP Authors
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
#include <app/data-model-provider/ActionReturnStatus.h>
#include <clusters/Thermostat/Metadata.h>
#include <lib/support/Assertions.h>
#include <protocols/interaction_model/StatusCode.h>

#include "Setpoint.h"
#include "Temperature.h"
#include "ThermostatCluster.h"

#include "Setpoints.h"

using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

DataModel::ActionReturnStatus ThermostatCluster::ReadSetpointAttribute(const DataModel::ReadAttributeRequest & request,
                                                                       AttributeValueEncoder & encoder)
{
    auto setpoints = GetSetpoints();
    switch (request.path.mAttributeId)
    {
    case OccupiedHeatingSetpoint::Id:
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(setpoints.occupiedRange.heating.Temperature());
    case OccupiedCoolingSetpoint::Id:
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(setpoints.occupiedRange.cooling.Temperature());
    case UnoccupiedHeatingSetpoint::Id:
        if (!setpoints.heatSupported || !setpoints.occupancySupported)
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(setpoints.unoccupiedRange.heating.Temperature());
    case UnoccupiedCoolingSetpoint::Id:
        if (!setpoints.coolSupported || !setpoints.occupancySupported)
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(setpoints.unoccupiedRange.cooling.Temperature());
    case AbsMinHeatSetpointLimit::Id:
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(setpoints.absoluteHeatLimits.minimum.Temperature());
    case AbsMaxHeatSetpointLimit::Id:
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(setpoints.absoluteHeatLimits.maximum.Temperature());
    case AbsMinCoolSetpointLimit::Id:
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(setpoints.absoluteCoolLimits.minimum.Temperature());
    case AbsMaxCoolSetpointLimit::Id:
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(setpoints.absoluteCoolLimits.maximum.Temperature());
    case MinHeatSetpointLimit::Id:
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(setpoints.userHeatLimits.minimum.Temperature());
    case MaxHeatSetpointLimit::Id:
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(setpoints.userHeatLimits.maximum.Temperature());
    case MinCoolSetpointLimit::Id:
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(setpoints.userCoolLimits.minimum.Temperature());
    case MaxCoolSetpointLimit::Id:
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(setpoints.userCoolLimits.maximum.Temperature());
    case MinSetpointDeadBand::Id: {
        if (!setpoints.autoSupported)
        {
            return Status::UnsupportedAttribute;
        }
        auto deadband = static_cast<int8_t>(setpoints.deadBand / 10);
        return encoder.Encode(deadband);
    }
    default:
        ChipLogError(Zcl, "Unsupported Setpoint Attribute:" ChipLogFormatMEI, ChipLogValueMEI(request.path.mAttributeId));
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus ThermostatCluster::HandleSetpointChange(Setpoints & setpoints, const AttributeId attributeId,
                                                                      temperature value, SetpointAttributes & changedAttributes)
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
    case MinSetpointDeadBand::Id:
        return Status::Success;
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus ThermostatCluster::SetpointRaiseLower(const Commands::SetpointRaiseLower::DecodableType & commandData)
{
    auto & mode    = commandData.mode;
    int16_t amount = static_cast<int16_t>(commandData.amount * 10);

    Setpoints currentSetpoints = GetSetpoints();
    Setpoints setpoints        = currentSetpoints;

    OccupancyBitmap isOccupied = IsOccupied()
        ? OccupancyBitmap::kOccupied
        : OccupancyBitmap(0);

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
    auto status = setpoints.ChangeRange(range, heat, cool, Setpoints::ClampMode::kClamp, changedAttributes);
    if (status != Status::Success)
    {
        return status;
    }
    return SaveSetpoints(setpoints, changedAttributes);
}

Protocols::InteractionModel::Status ThermostatCluster::SaveSetpoint(Setpoint & oldSetpoint, Setpoint & newSetpoint)
{
    VerifyOrReturnValue(oldSetpoint.AttributeId() == newSetpoint.AttributeId(), Status::InvalidCommand);
    VerifyOrReturnValue(oldSetpoint.Temperature() != newSetpoint.Temperature(), Status::Success);

    auto status = mDelegate->SaveSetpoint(oldSetpoint, newSetpoint);
    if (status != Status::Success)
    {
        return status;
    }
    if (mFeatures.Has(Feature::kEvents))
    {
        GenerateSetpointEvent(oldSetpoint.AttributeId(), oldSetpoint.Temperature(), newSetpoint.Temperature());
    }
    oldSetpoint.SetTemperature(newSetpoint.Temperature());
    NotifyAttributeChanged(oldSetpoint.AttributeId());
    return Status::Success;
}

DataModel::ActionReturnStatus ThermostatCluster::SaveSetpoints(Setpoints & setpoints, SetpointAttributes changedAttributes)
{
    if (!setpoints.Valid())
    {
        return Status::ConstraintError;
    }
    Setpoints currentSetpoints = GetSetpoints();

    Status status = Status::Success;
    if (setpoints.heatSupported)
    {
        if (changedAttributes.Has(MinHeatSetpointLimit::Id) && setpoints.userHeatLimits.minimum.HasTemperature())
        {
            status = SaveSetpoint(currentSetpoints.userHeatLimits.minimum, setpoints.userHeatLimits.minimum);
            VerifyOrReturnValue(status == Status::Success, status);
        }
        if (changedAttributes.Has(MaxHeatSetpointLimit::Id) && setpoints.userHeatLimits.maximum.HasTemperature())
        {
            status = SaveSetpoint(currentSetpoints.userHeatLimits.maximum, setpoints.userHeatLimits.maximum);
            VerifyOrReturnValue(status == Status::Success, status);
        }
        if (changedAttributes.Has(OccupiedHeatingSetpoint::Id))
        {
            status = SaveSetpoint(currentSetpoints.occupiedRange.heating, setpoints.occupiedRange.heating);
            VerifyOrReturnValue(status == Status::Success, status);
        }
        if (setpoints.occupancySupported)
        {
            if (changedAttributes.Has(UnoccupiedHeatingSetpoint::Id))
            {
                status = SaveSetpoint(currentSetpoints.unoccupiedRange.heating, setpoints.unoccupiedRange.heating);
                VerifyOrReturnValue(status == Status::Success, status);
            }
        }
    }
    if (setpoints.coolSupported)
    {
        if (changedAttributes.Has(MinCoolSetpointLimit::Id) && setpoints.userCoolLimits.minimum.HasTemperature())
        {
            status = SaveSetpoint(currentSetpoints.userCoolLimits.minimum, setpoints.userCoolLimits.minimum);
            VerifyOrReturnValue(status == Status::Success, status);
        }
        if (changedAttributes.Has(MaxCoolSetpointLimit::Id) && setpoints.userCoolLimits.maximum.HasTemperature())
        {
            status = SaveSetpoint(currentSetpoints.userCoolLimits.maximum, setpoints.userCoolLimits.maximum);
            VerifyOrReturnValue(status == Status::Success, status);
        }
        if (changedAttributes.Has(OccupiedCoolingSetpoint::Id))
        {
            status = SaveSetpoint(currentSetpoints.occupiedRange.cooling, setpoints.occupiedRange.cooling);
            VerifyOrReturnValue(status == Status::Success, status);
        }
        if (setpoints.occupancySupported)
        {
            if (changedAttributes.Has(UnoccupiedCoolingSetpoint::Id))
            {
                status = SaveSetpoint(currentSetpoints.unoccupiedRange.cooling, setpoints.unoccupiedRange.cooling);
                VerifyOrReturnValue(status == Status::Success, status);
            }
        }
    }
    return Status::Success;
}

DataModel::ActionReturnStatus ThermostatCluster::ChangeSetpointAttribute(const AttributeId attributeId, temperature temp)
{
    Setpoints setpoints = GetSetpoints();
    SetpointAttributes changedAttributes;

    auto status = HandleSetpointChange(setpoints, attributeId, temp, changedAttributes);
    if (status == Status::Success)
    {
        status = SaveSetpoints(setpoints, changedAttributes);
    }
    return status;
}

Setpoints ThermostatCluster::GetSetpoints()
{
    Setpoints setpoints;
    setpoints.autoSupported      = mFeatures.Has(Feature::kAutoMode);
    setpoints.heatSupported      = mFeatures.Has(Feature::kHeating);
    setpoints.coolSupported      = mFeatures.Has(Feature::kCooling);
    setpoints.occupancySupported = mFeatures.Has(Feature::kOccupancy);
    if (mDelegate != nullptr)
    {
        mDelegate->LoadSetpoints(setpoints);
    }
    return setpoints;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
