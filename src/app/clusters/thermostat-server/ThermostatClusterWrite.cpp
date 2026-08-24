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

DataModel::ActionReturnStatus ThermostatClusterCore::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                    AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case LocalTemperatureCalibration::Id: {
        int16_t cal;
        ReturnErrorOnFailure(decoder.Decode(cal));
        if (cal < std::numeric_limits<int8_t>::min() || cal > std::numeric_limits<int8_t>::max())
        {
            ChipLogError(Zcl, "Invalid value for LocalTemperatureCalibration: %d", cal);
            return Status::ConstraintError;
        }
        bool changed = false;
        if (auto err = mDelegate.SetLocalTemperatureCalibration(static_cast<int8_t>(cal), changed); err != Status::Success)
        {
            return err;
        }
        if (changed)
        {
            NotifyAttributeChanged(LocalTemperatureCalibration::Id);
        }
        return Status::Success;
    }
    case MinSetpointDeadBand::Id: {
        int16_t db;
        ReturnErrorOnFailure(decoder.Decode(db));
        if (db < kMinDeadBand || db > kMaxDeadBand)
        {
            ChipLogError(Zcl, "Invalid value for Deadband: %d", db);
            return Status::ConstraintError;
        }
        // Note: for backwards compatibility, writes to this attribute are allowed (as long as the value is valid) but ignored
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
    default:
        ChipLogError(Zcl, "Unsupported Attribute:" ChipLogFormatMEI, ChipLogValueMEI(request.path.mAttributeId));
        return Status::UnsupportedAttribute;
    }
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
