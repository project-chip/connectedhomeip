/**
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "ThermostatCluster.h"
#include "app/clusters/thermostat-server/Temperature.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>

#include <app/persistence/AttributePersistence.h>
#include <clusters/Thermostat/Metadata.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

DataModel::ActionReturnStatus ThermostatCluster::WriteNonAtomicAttribute(const DataModel::WriteAttributeRequest & request,
                                                                         AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case LocalTemperatureCalibration::Id:
        // TODO: implement local temperature calibration
        return Status::UnsupportedAttribute;
    case OccupiedCoolingSetpoint::Id:
    case OccupiedHeatingSetpoint::Id:
    case UnoccupiedCoolingSetpoint::Id:
    case UnoccupiedHeatingSetpoint::Id:
    case MinHeatSetpointLimit::Id:
    case MaxHeatSetpointLimit::Id:
    case MinCoolSetpointLimit::Id:
    case MaxCoolSetpointLimit::Id: {
        temperature setpoint;
        ReturnErrorOnFailure(decoder.Decode(setpoint));
        return ChangeSetpointAttribute(request.path.mAttributeId, setpoint);
    }
    case MinSetpointDeadBand::Id: {
        int16_t db;
        ReturnErrorOnFailure(decoder.Decode(db));
        if (db < kMinDeadBand || db > kMaxDeadBand)
        {
            ChipLogError(Zcl, "Invalid value for Deadband: %d", db);
            return Status::ConstraintError;
        }
        return Status::Success;
    }
    case RemoteSensing::Id: {
        BitMask<RemoteSensingBitmap> valueRemoteSensing;
        ReturnErrorOnFailure(decoder.Decode(valueRemoteSensing));
        if (mFeatures.Has(Feature::kLocalTemperatureNotExposed))
        {
            if (valueRemoteSensing.Has(RemoteSensingBitmap::kLocalTemperature))
            {
                return Status::ConstraintError;
            }
        }
        bool changed = false;
        if (auto err = mDelegate.SetRemoteSensing(valueRemoteSensing, changed); err != Status::Success)
        {
            return err;
        }
        if (changed)
        {
            NotifyAttributeChanged(RemoteSensing::Id);
        }
        return Status::Success;
    }
    case ControlSequenceOfOperation::Id:
        // Per spec, writes to this attribute are ignored, but success is always returned for backwards compatibility reasons
        return Status::Success;
    case SystemMode::Id: {
        SystemModeEnum requestedSystemMode;
        ReturnErrorOnFailure(decoder.Decode(requestedSystemMode));
        if (EnsureKnownEnumValue(requestedSystemMode) == SystemModeEnum::kUnknownEnumValue)
        {
            ChipLogError(Zcl, "Invalid value for SystemMode: %d", to_underlying(requestedSystemMode));
            return Status::InvalidValue;
        }
        return SetSystemMode(requestedSystemMode);
    }
    case TemperatureSetpointHold::Id: {
        TemperatureSetpointHoldEnum requestedTemperatureSetpointHold;
        ReturnErrorOnFailure(decoder.Decode(requestedTemperatureSetpointHold));
        if (EnsureKnownEnumValue(requestedTemperatureSetpointHold) == TemperatureSetpointHoldEnum::kUnknownEnumValue)
        {
            ChipLogError(Zcl, "Invalid value for TemperatureSetpointHold: %d", to_underlying(requestedTemperatureSetpointHold));
            return Status::InvalidValue;
        }
        bool changed = false;
        if (auto err = mDelegate.SetTemperatureSetpointHold(requestedTemperatureSetpointHold, changed); err != Status::Success)
        {
            return err;
        }
        if (changed)
        {
            NotifyAttributeChanged(TemperatureSetpointHold::Id);
        }
        return Status::Success;
    }
    case TemperatureSetpointHoldDuration::Id: {
        DataModel::Nullable<uint16_t> requestedTemperatureSetpointHoldDuration;
        ReturnErrorOnFailure(decoder.Decode(requestedTemperatureSetpointHoldDuration));
        if (!requestedTemperatureSetpointHoldDuration.IsNull() &&
            requestedTemperatureSetpointHoldDuration.Value() > kMaxTemperatureSetpointHoldDurationMin)
        {
            return Status::InvalidValue;
        }
        bool changed = false;
        if (auto err = mDelegate.SetTemperatureSetpointHoldDuration(requestedTemperatureSetpointHoldDuration, changed);
            err != Status::Success)
        {
            return err;
        }
        if (changed)
        {
            NotifyAttributeChanged(TemperatureSetpointHoldDuration::Id);
        }
        return Status::Success;
    }
    default:
        ChipLogError(Zcl, "Unsupported Attribute:" ChipLogFormatMEI, ChipLogValueMEI(request.path.mAttributeId));
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus ThermostatCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & decoder)
{
    auto & subjectDescriptor = decoder.GetSubjectDescriptor();
    if (mAtomicWriteSession.InAtomicWrite(subjectDescriptor))
    {
        ChipLogError(Zcl, "Can not write to non-atomic attribute " ChipLogFormatMEI " during atomic write",
                     ChipLogValueMEI(request.path.mAttributeId));
        return Status::InvalidInState;
    }
    return WriteNonAtomicAttribute(request, decoder);
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
