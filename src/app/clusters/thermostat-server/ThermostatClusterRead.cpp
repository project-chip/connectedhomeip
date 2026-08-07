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

DataModel::ActionReturnStatus ThermostatCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{
    bool localTemperatureNotExposedSupported = mFeatures.Has(Feature::kLocalTemperatureNotExposed);
    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "ThermostatDelegate is null");
        return Status::Failure;
    }

    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(Thermostat::kRevision);
    case Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case LocalTemperature::Id:
        if (localTemperatureNotExposedSupported)
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(GetLocalTemperature());
    case SystemMode::Id:
        return encoder.Encode(GetSystemMode());
    case ThermostatRunningMode::Id:
        return encoder.Encode(GetRunningMode());
    case RemoteSensing::Id:
        if (localTemperatureNotExposedSupported)
        {
            BitMask<RemoteSensingBitmap> valueRemoteSensing = mDelegate->GetRemoteSensing();
            valueRemoteSensing.Clear(RemoteSensingBitmap::kLocalTemperature);
            return encoder.Encode(valueRemoteSensing);
        }
        return encoder.Encode(mDelegate->GetRemoteSensing());
    case ControlSequenceOfOperation::Id:
        return encoder.Encode(mDelegate->GetControlSequenceOfOperation());
    case LocalTemperatureCalibration::Id:
        return encoder.Encode(mDelegate->GetLocalTemperatureCalibration());
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
        return encoder.Encode(mDelegate->GetTemperatureSetpointHold());
    case TemperatureSetpointHoldDuration::Id:
        return encoder.Encode(mDelegate->GetTemperatureSetpointHoldDuration());
    case Schedules::Id: {
        return encoder.EncodeList([](const auto & enc) -> CHIP_ERROR { return CHIP_NO_ERROR; });
    }
    break;
    case SetpointHoldExpiryTimestamp::Id: {
        return encoder.Encode(mDelegate->GetSetpointHoldExpiryTimestamp());
    }
    break;
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
