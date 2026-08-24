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

#include "Setpoints.h"
#include "Temperature.h"
#include "ThermostatClusterCore.h"
#include "ThermostatClusterSetpointsBase.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <lib/support/BitMask.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatClusterCore;

class ThermostatCoolingSetpoints
{
public:
    class Delegate
    {
    public:
        Delegate()          = default;
        virtual ~Delegate() = default;

        virtual Protocols::InteractionModel::Status GetOccupiedCoolingSetpoint(temperature & occupiedCoolingSetpoint) const = 0;
        virtual Protocols::InteractionModel::Status SetOccupiedCoolingSetpoint(temperature occupiedCoolingSetpoint,
                                                                               bool & changed)                              = 0;

        virtual Protocols::InteractionModel::Status GetAbsMinCoolSetpointLimit(temperature & absMinCoolSetpointLimit) const;
        virtual Protocols::InteractionModel::Status GetAbsMaxCoolSetpointLimit(temperature & absMaxCoolSetpointLimit) const;

        virtual Protocols::InteractionModel::Status GetMinCoolSetpointLimit(temperature & minCoolSetpointLimit) const;
        virtual Protocols::InteractionModel::Status SetMinCoolSetpointLimit(temperature minCoolSetpointLimit, bool & changed);

        virtual Protocols::InteractionModel::Status GetMaxCoolSetpointLimit(temperature & maxCoolSetpointLimit) const;
        virtual Protocols::InteractionModel::Status SetMaxCoolSetpointLimit(temperature maxCoolSetpointLimit, bool & changed);

        virtual Protocols::InteractionModel::Status GetUnoccupiedCoolingSetpoint(temperature & unoccupiedCoolingSetpoint) const;
        virtual Protocols::InteractionModel::Status SetUnoccupiedCoolingSetpoint(temperature unoccupiedCoolingSetpoint,
                                                                                 bool & changed);
    };

    ThermostatCoolingSetpoints(const ThermostatSetpointsBase & setpoints, Delegate & delegate) :
        mSetpoints(setpoints), mDelegate(delegate)
    {}

    Protocols::InteractionModel::Status LoadSetpoints(Setpoints & setpoints);
    DataModel::ActionReturnStatus SaveSetpoints(const Setpoints & currentSetpoints, const Setpoints & changedSetpoints,
                                                SetpointAttributes & changedAttributes);

    bool HasAttribute(AttributeId attributeId);
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder);
    std::optional<DataModel::ActionReturnStatus> ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder);
    std::optional<DataModel::ActionReturnStatus> WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & decoder, Setpoints & setpoints,
                                                                SetpointAttributes & changedAttributes);

private:
    const ThermostatSetpointsBase & mSetpoints;
    Delegate & mDelegate;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
