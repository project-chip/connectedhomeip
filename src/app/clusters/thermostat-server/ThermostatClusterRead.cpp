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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>

#include <clusters/Thermostat/Metadata.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

DataModel::ActionReturnStatus ThermostatCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(Thermostat::kRevision);
    case Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case LocalTemperature::Id:
        if (mFeatures.Has(Feature::kLocalTemperatureNotExposed))
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mLocalTemperature);
    case OutdoorTemperature::Id:
        // TODO: implement outdoor temperature
        return Status::UnsupportedAttribute;
    case Occupancy::Id:
        // TODO: implement occupancy
        return Status::UnsupportedAttribute;
    case SystemMode::Id:
        return encoder.Encode(mSystemMode);
    case ThermostatRunningMode::Id:
        return encoder.Encode(mRunningMode);
    case RemoteSensing::Id: {
        BitMask<RemoteSensingBitmap> valueRemoteSensing = mRemoteSensing;
        if (mFeatures.Has(Feature::kLocalTemperatureNotExposed))
        {
            valueRemoteSensing.ClearAll();
        }
        return encoder.Encode(valueRemoteSensing);
    }
    case ControlSequenceOfOperation::Id:
        return encoder.Encode(mControlSequenceOfOperation);
    case LocalTemperatureCalibration::Id:
        return encoder.Encode(mLocalTemperatureCalibration);
    case OccupiedHeatingSetpoint::Id:
    case OccupiedCoolingSetpoint::Id:
    case UnoccupiedHeatingSetpoint::Id:
    case UnoccupiedCoolingSetpoint::Id:
    case AbsMinHeatSetpointLimit::Id:
    case AbsMaxHeatSetpointLimit::Id:
    case AbsMinCoolSetpointLimit::Id:
    case AbsMaxCoolSetpointLimit::Id:
    case MinHeatSetpointLimit::Id:
    case MaxHeatSetpointLimit::Id:
    case MinCoolSetpointLimit::Id:
    case MaxCoolSetpointLimit::Id:
    case MinSetpointDeadBand::Id:
        return ReadSetpointAttribute(request, encoder);
    case TemperatureSetpointHold::Id:
        return encoder.Encode(mTemperatureSetpointHold);
    case TemperatureSetpointHoldDuration::Id:
        return encoder.Encode(mTemperatureSetpointHoldDuration);
    case ThermostatRunningState::Id:
        // TODO: implement thermostat running state
        return Status::UnsupportedAttribute;
    case SetpointChangeSource::Id:
    case SetpointChangeAmount::Id:
    case SetpointChangeSourceTimestamp::Id:
        // TODO: implement setpoint change attributes
        return Status::UnsupportedAttribute;
    case NumberOfSchedules::Id:
    case NumberOfScheduleTransitions::Id:
    case NumberOfScheduleTransitionPerDay::Id:
        // TODO: implement number of schedules
        return Status::UnsupportedAttribute;
    case ActiveScheduleHandle::Id:
        // TODO: implement active schedule handle
        return Status::UnsupportedAttribute;
||||||| parent of 5217deb1c7d (Move setpoint loading/saving to core delegate)
    }
=======
>>>>>>> 5217deb1c7d (Move setpoint loading/saving to core delegate)
    case Schedules::Id: {
        // TODO: Implement schedule list
        return encoder.EncodeList([](const auto & enc) -> CHIP_ERROR { return CHIP_NO_ERROR; });
    }
    case SetpointHoldExpiryTimestamp::Id: {
        ReturnErrorOnFailure(encoder.Encode(mSetpointHoldExpiryTimestamp));
    }
    default:
        ChipLogError(Zcl, "Unsupported Attribute:" ChipLogFormatMEI, ChipLogValueMEI(request.path.mAttributeId));
        return Status::UnsupportedAttribute;
    }
    return Status::Success;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
