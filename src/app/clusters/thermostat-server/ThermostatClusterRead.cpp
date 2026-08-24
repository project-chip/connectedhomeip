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

DataModel::ActionReturnStatus ThermostatClusterCore::ReadAttribute(const DataModel::ReadAttributeRequest & request,
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
        return encoder.Encode(GetLocalTemperature());
    case SystemMode::Id:
        return encoder.Encode(GetSystemMode());
    case ThermostatRunningMode::Id:
        return encoder.Encode(GetRunningMode());
    case RemoteSensing::Id:
        if (mFeatures.Has(Feature::kLocalTemperatureNotExposed))
        {
            BitMask<RemoteSensingBitmap> valueRemoteSensing = mDelegate.GetRemoteSensing();
            valueRemoteSensing.Clear(RemoteSensingBitmap::kLocalTemperature);
            return encoder.Encode(valueRemoteSensing);
        }
        return encoder.Encode(mDelegate.GetRemoteSensing());
    case ControlSequenceOfOperation::Id:
        return encoder.Encode(mDelegate.GetControlSequenceOfOperation());
    case LocalTemperatureCalibration::Id:
        return encoder.Encode(mDelegate.GetLocalTemperatureCalibration());

    case Schedules::Id: {
        // TODO: Implement schedule list
        return encoder.EncodeList([](const auto & enc) -> CHIP_ERROR { return CHIP_NO_ERROR; });
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
