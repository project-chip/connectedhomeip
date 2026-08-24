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
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/Thermostat/Metadata.h>
#include <lib/support/Assertions.h>
#include <protocols/interaction_model/StatusCode.h>

#include "Setpoint.h"
#include "Temperature.h"
#include "ThermostatClusterCoolingSetpoints.h"

#include "Setpoints.h"

using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Commands;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

Status ThermostatCoolingSetpoints::Delegate::GetAbsMinCoolSetpointLimit(temperature & absMinCoolSetpointLimit) const
{
    return Status::UnsupportedAttribute;
};

Status ThermostatCoolingSetpoints::Delegate::GetAbsMaxCoolSetpointLimit(temperature & absMaxCoolSetpointLimit) const
{
    return Status::UnsupportedAttribute;
};

Status ThermostatCoolingSetpoints::Delegate::GetMinCoolSetpointLimit(temperature & minCoolSetpointLimit) const
{
    return Status::UnsupportedAttribute;
};

Status ThermostatCoolingSetpoints::Delegate::SetMinCoolSetpointLimit(temperature minCoolSetpointLimit, bool & changed)
{
    return Status::UnsupportedAttribute;
};

Status ThermostatCoolingSetpoints::Delegate::GetMaxCoolSetpointLimit(temperature & maxCoolSetpointLimit) const
{
    return Status::UnsupportedAttribute;
};

Status ThermostatCoolingSetpoints::Delegate::SetMaxCoolSetpointLimit(temperature maxCoolSetpointLimit, bool & changed)
{
    return Status::UnsupportedAttribute;
};

Status ThermostatCoolingSetpoints::Delegate::GetUnoccupiedCoolingSetpoint(temperature & unoccupiedCoolingSetpoint) const
{
    return Status::UnsupportedAttribute;
};

Status ThermostatCoolingSetpoints::Delegate::SetUnoccupiedCoolingSetpoint(temperature unoccupiedCoolingSetpoint, bool & changed)
{
    return Status::UnsupportedAttribute;
};

bool ThermostatCoolingSetpoints::HasAttribute(AttributeId attributeId)
{
    switch (attributeId)
    {
    case OccupiedCoolingSetpoint::Id:
    case AbsMinCoolSetpointLimit::Id:
    case AbsMaxCoolSetpointLimit::Id:
    case MinCoolSetpointLimit::Id:
    case MaxCoolSetpointLimit::Id:
        return true;
    default:
        return false;
    }
}

std::optional<DataModel::ActionReturnStatus>
ThermostatCoolingSetpoints::ReadAttribute(const DataModel::ReadAttributeRequest & request, AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case OccupiedCoolingSetpoint::Id: {
        temperature coolingSetpoint;
        auto status = mDelegate.GetOccupiedCoolingSetpoint(coolingSetpoint);
        VerifyOrReturnValue(status == Status::Success, status);
        return encoder.Encode(coolingSetpoint);
    }
    case MinCoolSetpointLimit::Id: {
        temperature minCoolSetpointLimit;
        auto status = mDelegate.GetMinCoolSetpointLimit(minCoolSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        return encoder.Encode(minCoolSetpointLimit);
    }
    case MaxCoolSetpointLimit::Id: {
        temperature maxCoolSetpointLimit;
        auto status = mDelegate.GetMaxCoolSetpointLimit(maxCoolSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        return encoder.Encode(maxCoolSetpointLimit);
    }
    case AbsMinCoolSetpointLimit::Id: {
        temperature absMinCoolSetpointLimit;
        auto status = mDelegate.GetAbsMinCoolSetpointLimit(absMinCoolSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        return encoder.Encode(absMinCoolSetpointLimit);
    }
    case AbsMaxCoolSetpointLimit::Id: {
        temperature absMaxCoolSetpointLimit;
        auto status = mDelegate.GetAbsMaxCoolSetpointLimit(absMaxCoolSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        return encoder.Encode(absMaxCoolSetpointLimit);
    }

    default:
        return std::nullopt;
    }
}

Protocols::InteractionModel::Status ThermostatCoolingSetpoints::LoadSetpoints(Setpoints & setpoints)
{
    auto & optionalAttributes = mSetpoints.GetOptionalAttributes();
    if (optionalAttributes.AbsMinCoolSetpointLimit)
    {
        temperature absMinCoolSetpointLimit;
        auto status = mDelegate.GetAbsMinCoolSetpointLimit(absMinCoolSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        setpoints.absoluteCoolLimits.minimum.SetTemperature(absMinCoolSetpointLimit);
    }
    else
    {
        setpoints.absoluteCoolLimits.minimum.SetTemperature(kDefaultAbsMinCoolSetpointLimit);
    }

    if (optionalAttributes.AbsMaxCoolSetpointLimit)
    {
        temperature absMaxCoolSetpointLimit;
        auto status = mDelegate.GetAbsMaxCoolSetpointLimit(absMaxCoolSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        setpoints.absoluteCoolLimits.maximum.SetTemperature(absMaxCoolSetpointLimit);
    }
    else
    {
        setpoints.absoluteCoolLimits.maximum.SetTemperature(kDefaultAbsMaxCoolSetpointLimit);
    }

    if (optionalAttributes.MinCoolSetpointLimit)
    {
        temperature minCoolSetpointLimit;
        auto status = mDelegate.GetMinCoolSetpointLimit(minCoolSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        setpoints.userCoolLimits.minimum.SetTemperature(minCoolSetpointLimit);
    }

    if (optionalAttributes.MaxCoolSetpointLimit)
    {
        temperature maxCoolSetpointLimit;
        auto status = mDelegate.GetMaxCoolSetpointLimit(maxCoolSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        setpoints.userCoolLimits.maximum.SetTemperature(maxCoolSetpointLimit);
    }

    temperature occupiedCoolingSetpoint;
    auto status = mDelegate.GetOccupiedCoolingSetpoint(occupiedCoolingSetpoint);
    VerifyOrReturnValue(status == Status::Success, status);
    setpoints.occupiedRange.cooling.SetTemperature(occupiedCoolingSetpoint);
    return Status::Success;
}

DataModel::ActionReturnStatus ThermostatCoolingSetpoints::SaveSetpoints(const Setpoints & currentSetpoints,
                                                                        const Setpoints & changedSetpoints,
                                                                        SetpointAttributes & changedAttributes)
{
    if (!changedSetpoints.Valid())
    {
        return Status::ConstraintError;
    }

    Status status = Status::Success;

    if (changedAttributes.Has(MinCoolSetpointLimit::Id) && changedSetpoints.userCoolLimits.minimum.HasTemperature())
    {
        bool changed;
        status = mDelegate.SetMinCoolSetpointLimit(changedSetpoints.userCoolLimits.minimum.Temperature(), changed);
        VerifyOrReturnValue(status == Status::Success, status);
        if (!changed)
        {
            changedAttributes.Clear(MinCoolSetpointLimit::Id);
        }
        else
        {
            mSetpoints.GenerateSetpointEvent(MinCoolSetpointLimit::Id, currentSetpoints.userCoolLimits.minimum.Temperature(),
                                             changedSetpoints.userCoolLimits.minimum.Temperature());
        }
    }
    if (changedAttributes.Has(MaxCoolSetpointLimit::Id) && changedSetpoints.userCoolLimits.maximum.HasTemperature())
    {
        bool changed;
        status = mDelegate.SetMaxCoolSetpointLimit(changedSetpoints.userCoolLimits.maximum.Temperature(), changed);
        VerifyOrReturnValue(status == Status::Success, status);
        if (!changed)
        {
            changedAttributes.Clear(MaxCoolSetpointLimit::Id);
        }
        else
        {
            mSetpoints.GenerateSetpointEvent(MaxCoolSetpointLimit::Id, currentSetpoints.userCoolLimits.maximum.Temperature(),
                                             changedSetpoints.userCoolLimits.maximum.Temperature());
        }
    }
    if (changedAttributes.Has(OccupiedCoolingSetpoint::Id) && changedSetpoints.occupiedRange.cooling.HasTemperature())
    {
        bool changed;
        status = mDelegate.SetOccupiedCoolingSetpoint(changedSetpoints.occupiedRange.cooling.Temperature(), changed);
        VerifyOrReturnValue(status == Status::Success, status);
        if (!changed)
        {
            changedAttributes.Clear(OccupiedCoolingSetpoint::Id);
        }
        else
        {
            mSetpoints.GenerateSetpointEvent(OccupiedCoolingSetpoint::Id, currentSetpoints.occupiedRange.cooling.Temperature(),
                                             changedSetpoints.occupiedRange.cooling.Temperature());
        }
    }
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus>
ThermostatCoolingSetpoints::WriteAttribute(const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder,
                                           Setpoints & setpoints, SetpointAttributes & changedAttributes)
{
    if (!HasAttribute(request.path.mAttributeId))
    {
        return std::nullopt;
    }

    temperature setpoint;
    ReturnErrorOnFailure(decoder.Decode(setpoint));

    switch (request.path.mAttributeId)
    {
    case OccupiedCoolingSetpoint::Id:
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeRangeCooling(setpoints.occupiedRange, setpoint, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case UnoccupiedCoolingSetpoint::Id:
        if (!setpoints.occupancySupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeRangeCooling(setpoints.unoccupiedRange, setpoint, Setpoints::ClampMode::kDontClamp,
                                            changedAttributes);
    case MinCoolSetpointLimit::Id:
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeLimitMinimum(setpoints.userCoolLimits, setpoints.absoluteCoolLimits, setpoint, changedAttributes);
    case MaxCoolSetpointLimit::Id:
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeLimitMaximum(setpoints.userCoolLimits, setpoints.absoluteCoolLimits, setpoint, changedAttributes);
    default:
        return std::nullopt;
    }
}

CHIP_ERROR ThermostatCoolingSetpoints::Attributes(const ConcreteClusterPath & path,
                                                  ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { HasAttribute(OccupiedCoolingSetpoint::Id), OccupiedCoolingSetpoint::kMetadataEntry },
        { HasAttribute(UnoccupiedCoolingSetpoint::Id), UnoccupiedCoolingSetpoint::kMetadataEntry },
        { HasAttribute(AbsMinCoolSetpointLimit::Id), AbsMinCoolSetpointLimit::kMetadataEntry },
        { HasAttribute(AbsMaxCoolSetpointLimit::Id), AbsMaxCoolSetpointLimit::kMetadataEntry },
        { HasAttribute(MinCoolSetpointLimit::Id), MinCoolSetpointLimit::kMetadataEntry },
        { HasAttribute(MaxCoolSetpointLimit::Id), MaxCoolSetpointLimit::kMetadataEntry },
        { HasAttribute(SetpointChangeSource::Id), SetpointChangeSource::kMetadataEntry },
        { HasAttribute(SetpointChangeAmount::Id), SetpointChangeAmount::kMetadataEntry },
        { HasAttribute(SetpointChangeSourceTimestamp::Id), SetpointChangeSourceTimestamp::kMetadataEntry },
    };

    return AppendOptionalAttributes(builder, Span(optionalAttributes));
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
