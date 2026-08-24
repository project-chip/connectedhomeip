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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatClusterCore;

class ThermostatHold
{
public:
    class Delegate
    {
    public:
        virtual ~Delegate() = default;

        virtual TemperatureSetpointHoldEnum GetTemperatureSetpointHold() const                 = 0;
        virtual Protocols::InteractionModel::Status SetTemperatureSetpointHold(TemperatureSetpointHoldEnum temperatureSetpointHold,
                                                                           bool & changed) = 0;

        virtual DataModel::Nullable<uint16_t> GetTemperatureSetpointHoldDuration() const       = 0;
        virtual Protocols::InteractionModel::Status
        SetTemperatureSetpointHoldDuration(DataModel::Nullable<uint16_t> temperatureSetpointHoldDuration,
                                           bool & changed) = 0;

        virtual DataModel::Nullable<uint32_t> GetSetpointHoldExpiryTimestamp() const = 0;
        virtual Protocols::InteractionModel::Status
        SetSetpointHoldExpiryTimestamp(DataModel::Nullable<uint32_t> setpointHoldExpiryTimestamp,
                                           bool & changed) = 0;

    };

    ThermostatHold(ThermostatClusterCore & cluster, Delegate & delegate) : mCluster(cluster), mDelegate(delegate) {}

    std::optional<DataModel::ActionReturnStatus> ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder);
    std::optional<DataModel::ActionReturnStatus> WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & decoder);

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder);
private:
    ThermostatClusterCore & mCluster;
    Delegate & mDelegate;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
