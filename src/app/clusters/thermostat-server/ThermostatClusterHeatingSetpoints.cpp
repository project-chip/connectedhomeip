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
#include "ThermostatClusterHeatingSetpoints.h"

#include "Setpoints.h"
#include "app/clusters/thermostat-server/ThermostatClusterAttributes.h"

using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Commands;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

Status ThermostatHeatingSetpoints::Delegate::GetUnoccupiedHeatingSetpoint(temperature & unoccupiedHeatingSetpoint) const {
    return Status::UnsupportedAttribute;
};

Status ThermostatHeatingSetpoints::Delegate::SetUnoccupiedHeatingSetpoint(temperature unoccupiedHeatingSetpoint, bool & changed) {
    return Status::UnsupportedAttribute;
};

Status ThermostatHeatingSetpoints::Delegate::GetAbsMinHeatSetpointLimit(temperature & absMinHeatSetpointLimit) const {
    return Status::UnsupportedAttribute;
};
Status ThermostatHeatingSetpoints::Delegate::GetAbsMaxHeatSetpointLimit(temperature & absMaxHeatSetpointLimit) const {
    return Status::UnsupportedAttribute;
};

Status ThermostatHeatingSetpoints::Delegate::GetMinHeatSetpointLimit(temperature & minHeatSetpointLimit) const {
    return Status::UnsupportedAttribute;
};
Status ThermostatHeatingSetpoints::Delegate::SetMinHeatSetpointLimit(temperature minHeatSetpointLimit, bool & changed) {
    return Status::UnsupportedAttribute;
};

Status ThermostatHeatingSetpoints::Delegate::GetMaxHeatSetpointLimit(temperature & maxHeatSetpointLimit) const {
    return Status::UnsupportedAttribute;
};
Status ThermostatHeatingSetpoints::Delegate::SetMaxHeatSetpointLimit(temperature maxHeatSetpointLimit, bool & changed) {
    return Status::UnsupportedAttribute;
};


bool ThermostatHeatingSetpoints::HasAttribute(AttributeId attributeId) {
    switch (attributeId)
    {
    case OccupiedHeatingSetpoint::Id:
    case AbsMinHeatSetpointLimit::Id:
    case AbsMaxHeatSetpointLimit::Id:
    case MinHeatSetpointLimit::Id:
    case MaxHeatSetpointLimit::Id:
        return true;
    default:
        return false;
    }
}

Protocols::InteractionModel::Status ThermostatHeatingSetpoints::LoadSetpoints(Setpoints & setpoints) {
    OptionalAttributes attributes = mSetpoints.OptionalAttributes();

    if (attributes.AbsMinHeatSetpointLimit)
    {
        temperature absMinHeatSetpointLimit;
        auto status = mDelegate.GetAbsMinHeatSetpointLimit(absMinHeatSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        setpoints.absoluteHeatLimits.minimum.SetTemperature(absMinHeatSetpointLimit);
    } else {
        setpoints.absoluteHeatLimits.minimum.SetTemperature(kDefaultAbsMinHeatSetpointLimit);
    }
   
    if (attributes.AbsMaxHeatSetpointLimit)
    {
        temperature absMaxHeatSetpointLimit;
        auto status = mDelegate.GetAbsMaxHeatSetpointLimit(absMaxHeatSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        setpoints.absoluteHeatLimits.maximum.SetTemperature(absMaxHeatSetpointLimit);
    } else {
        setpoints.absoluteHeatLimits.maximum.SetTemperature(kDefaultAbsMaxHeatSetpointLimit);
    }
    
    if (attributes.MinHeatSetpointLimit)
    {
        temperature minHeatSetpointLimit;
        auto status = mDelegate.GetMinHeatSetpointLimit(minHeatSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        setpoints.userHeatLimits.minimum.SetTemperature(minHeatSetpointLimit);
    } 

    if (attributes.MaxHeatSetpointLimit)
    {
        temperature maxHeatSetpointLimit;
        auto status = mDelegate.GetMaxHeatSetpointLimit(maxHeatSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        setpoints.userHeatLimits.maximum.SetTemperature(maxHeatSetpointLimit);
    }
    
    temperature occupiedHeatingSetpoint;
    auto status = mDelegate.GetOccupiedHeatingSetpoint(occupiedHeatingSetpoint);
    VerifyOrReturnValue(status == Status::Success, status);
    setpoints.occupiedRange.heating.SetTemperature(occupiedHeatingSetpoint);

    if (setpoints.occupancySupported)
    {
        temperature unoccupiedHeatingSetpoint;
        status = mDelegate.GetUnoccupiedHeatingSetpoint(unoccupiedHeatingSetpoint);
        VerifyOrReturnValue(status == Status::Success, status);
        setpoints.unoccupiedRange.heating.SetTemperature(unoccupiedHeatingSetpoint);
    }
    return Status::Success;
}

DataModel::ActionReturnStatus ThermostatHeatingSetpoints::SaveSetpoints(const Setpoints & currentSetpoints, const Setpoints & changedSetpoints, SetpointAttributes & changedAttributes)
{
    if (!changedSetpoints.Valid())
    {
        return Status::ConstraintError;
    }

    Status status = Status::Success;
   
    if (changedAttributes.Has(MinHeatSetpointLimit::Id) && changedSetpoints.userHeatLimits.minimum.HasTemperature())
    {
        bool changed;
        status = mDelegate.SetMinHeatSetpointLimit(changedSetpoints.userHeatLimits.minimum.Temperature(), changed);
        VerifyOrReturnValue(status == Status::Success, status);
        if (!changed)
        {
            changedAttributes.Clear(MinHeatSetpointLimit::Id);
        } else {
            mSetpoints.GenerateSetpointEvent(MinHeatSetpointLimit::Id, currentSetpoints.userHeatLimits.minimum.Temperature(), changedSetpoints.userHeatLimits.minimum.Temperature());
        }
    }
    if (changedAttributes.Has(MaxHeatSetpointLimit::Id) && changedSetpoints.userHeatLimits.maximum.HasTemperature())
    {
        bool changed;
        status = mDelegate.SetMaxHeatSetpointLimit(changedSetpoints.userHeatLimits.maximum.Temperature(), changed);
        VerifyOrReturnValue(status == Status::Success, status);
        if (!changed)
        {
            changedAttributes.Clear(MaxHeatSetpointLimit::Id);
        } else {
            mSetpoints.GenerateSetpointEvent(MaxHeatSetpointLimit::Id, currentSetpoints.userHeatLimits.maximum.Temperature(), changedSetpoints.userHeatLimits.maximum.Temperature());
        }
    }
    if (changedAttributes.Has(OccupiedHeatingSetpoint::Id) && changedSetpoints.occupiedRange.heating.HasTemperature())
    {
        bool changed;
        status = mDelegate.SetOccupiedHeatingSetpoint(changedSetpoints.occupiedRange.heating.Temperature(), changed);
        VerifyOrReturnValue(status == Status::Success, status);
        if (!changed)
        {
            changedAttributes.Clear(OccupiedHeatingSetpoint::Id);
        } else {
            mSetpoints.GenerateSetpointEvent(OccupiedHeatingSetpoint::Id, currentSetpoints.occupiedRange.heating.Temperature(), changedSetpoints.occupiedRange.heating.Temperature());
        }
    }
    if (changedAttributes.Has(UnoccupiedHeatingSetpoint::Id) && changedSetpoints.unoccupiedRange.heating.HasTemperature())
    {
        bool changed;
        status = mDelegate.SetUnoccupiedHeatingSetpoint(changedSetpoints.unoccupiedRange.heating.Temperature(), changed);
        VerifyOrReturnValue(status == Status::Success, status);
        if (!changed)
        {
            changedAttributes.Clear(UnoccupiedHeatingSetpoint::Id);
        } else {
            mSetpoints.GenerateSetpointEvent(UnoccupiedHeatingSetpoint::Id, currentSetpoints.unoccupiedRange.heating.Temperature(), changedSetpoints.unoccupiedRange.heating.Temperature());
        }
    }

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> ThermostatHeatingSetpoints::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                                  AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case OccupiedHeatingSetpoint::Id:
    {
        temperature occupiedHeatingSetpoint;
        auto status = mDelegate.GetOccupiedHeatingSetpoint(occupiedHeatingSetpoint);
        VerifyOrReturnValue(status == Status::Success, status);
        return encoder.Encode(occupiedHeatingSetpoint);
    }
    case UnoccupiedHeatingSetpoint::Id:
    {
        temperature unoccupiedHeatingSetpoint;
        auto status = mDelegate.GetUnoccupiedHeatingSetpoint(unoccupiedHeatingSetpoint);
        VerifyOrReturnValue(status == Status::Success, status);
        return encoder.Encode(unoccupiedHeatingSetpoint);
    }
    case AbsMinHeatSetpointLimit::Id:
    {
        temperature absMinHeatSetpointLimit;
        auto status = mDelegate.GetAbsMinHeatSetpointLimit(absMinHeatSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        return encoder.Encode(absMinHeatSetpointLimit);
    }
    case AbsMaxHeatSetpointLimit::Id:
    {
        temperature absMaxHeatSetpointLimit;
        auto status = mDelegate.GetAbsMaxHeatSetpointLimit(absMaxHeatSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        return encoder.Encode(absMaxHeatSetpointLimit);
    }
    case MinHeatSetpointLimit::Id:
    {
        temperature minHeatSetpointLimit;
        auto status = mDelegate.GetMinHeatSetpointLimit(minHeatSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        return encoder.Encode(minHeatSetpointLimit);
    }
    case MaxHeatSetpointLimit::Id:
    {
        temperature maxHeatSetpointLimit;
        auto status = mDelegate.GetMaxHeatSetpointLimit(maxHeatSetpointLimit);
        VerifyOrReturnValue(status == Status::Success, status);
        return encoder.Encode(maxHeatSetpointLimit);
    }
    default:
        return std::nullopt;
    }
}

std::optional<DataModel::ActionReturnStatus> ThermostatHeatingSetpoints::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                                 AttributeValueDecoder & decoder, Setpoints & setpoints, SetpointAttributes & changedAttributes)
{
    if (!HasAttribute(request.path.mAttributeId))
    {
        return std::nullopt;
    }

    temperature setpoint;
    ReturnErrorOnFailure(decoder.Decode(setpoint));
    
    switch (request.path.mAttributeId)
    {
    case OccupiedHeatingSetpoint::Id:
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeRangeHeating(setpoints.occupiedRange, setpoint, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case UnoccupiedHeatingSetpoint::Id:
        if (!setpoints.occupancySupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeRangeHeating(setpoints.unoccupiedRange, setpoint, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case MinHeatSetpointLimit::Id:
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeLimitMinimum(setpoints.userHeatLimits, setpoints.absoluteHeatLimits, setpoint, changedAttributes);
    case MaxHeatSetpointLimit::Id:
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeLimitMaximum(setpoints.userHeatLimits, setpoints.absoluteHeatLimits, setpoint, changedAttributes);
    default:
        return std::nullopt;
    }
}

CHIP_ERROR ThermostatHeatingSetpoints::Attributes(const ConcreteClusterPath & path,
                                                  ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { HasAttribute(OccupiedHeatingSetpoint::Id), OccupiedHeatingSetpoint::kMetadataEntry },
        { HasAttribute(UnoccupiedHeatingSetpoint::Id), UnoccupiedHeatingSetpoint::kMetadataEntry },
        { HasAttribute(AbsMinHeatSetpointLimit::Id), AbsMinHeatSetpointLimit::kMetadataEntry },
        { HasAttribute(AbsMaxHeatSetpointLimit::Id), AbsMaxHeatSetpointLimit::kMetadataEntry },
        { HasAttribute(MinHeatSetpointLimit::Id), MinHeatSetpointLimit::kMetadataEntry },
        { HasAttribute(MaxHeatSetpointLimit::Id), MaxHeatSetpointLimit::kMetadataEntry },
    };

    return AppendOptionalAttributes(builder, Span(optionalAttributes));
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
