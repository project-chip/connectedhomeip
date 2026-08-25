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

#include "ThermostatClusterHold.h"
#include "ThermostatClusterCore.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/Thermostat/Metadata.h>
#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

CHIP_ERROR ThermostatHold::Delegate::Startup(ServerClusterContext & context)
{
    return CHIP_NO_ERROR;
}

void ThermostatHold::Delegate::Shutdown(ClusterShutdownType type) {}

CHIP_ERROR ThermostatHold::Startup(ServerClusterContext & context)
{
    return mDelegate.Startup(context);
}

void ThermostatHold::Shutdown(ClusterShutdownType type)
{
    mDelegate.Shutdown(type);
}

std::optional<DataModel::ActionReturnStatus> ThermostatHold::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                           AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case TemperatureSetpointHold::Id:
        return encoder.Encode(mDelegate.GetTemperatureSetpointHold());
    case TemperatureSetpointHoldDuration::Id:
        return encoder.Encode(mDelegate.GetTemperatureSetpointHoldDuration());
    case SetpointHoldExpiryTimestamp::Id:
        return encoder.Encode(mDelegate.GetSetpointHoldExpiryTimestamp());
    default:
        return std::nullopt;
    }
}

std::optional<DataModel::ActionReturnStatus> ThermostatHold::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                            AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
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
            mCluster.NotifyAttributeChanged(TemperatureSetpointHold::Id);
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
            mCluster.NotifyAttributeChanged(TemperatureSetpointHoldDuration::Id);
        }
        return Status::Success;
    }
    case SetpointHoldExpiryTimestamp::Id: {
        DataModel::Nullable<uint32_t> setpointHoldExpiryTimestamp;
        ReturnErrorOnFailure(decoder.Decode(setpointHoldExpiryTimestamp));
        bool changed = false;
        if (auto err = mDelegate.SetSetpointHoldExpiryTimestamp(setpointHoldExpiryTimestamp, changed); err != Status::Success)
        {
            return err;
        }
        if (changed)
        {
            mCluster.NotifyAttributeChanged(SetpointHoldExpiryTimestamp::Id);
        }
        return Status::Success;
    }
    default:
        return std::nullopt;
    }
}

CHIP_ERROR ThermostatHold::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    const auto & optionalAttributes                              = mCluster.GetOptionalAttributes();
    const AttributeListBuilder::OptionalAttributeEntry entries[] = {
        { optionalAttributes.TemperatureSetpointHold, TemperatureSetpointHold::kMetadataEntry },
        { optionalAttributes.TemperatureSetpointHoldDuration, TemperatureSetpointHoldDuration::kMetadataEntry },
        { optionalAttributes.SetpointHoldExpiryTimestamp, SetpointHoldExpiryTimestamp::kMetadataEntry },
    };

    return AppendOptionalAttributes(builder, Span(entries));
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
