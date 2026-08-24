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
#include "ThermostatClusterAttributes.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/OperationTypes.h>
#include <lib/support/BitMask.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatClusterCore;

class ThermostatSetpointsBase
{
public:
    ThermostatSetpointsBase(ThermostatClusterCore & cluster) : mCluster(cluster) {}
    virtual ~ThermostatSetpointsBase() = default;

    virtual Protocols::InteractionModel::Status LoadSetpoints(Setpoints & setpoints) = 0;

    const OptionalAttributes & OptionalAttributes() const;
    const BitFlags<Thermostat::Feature> & Features() const;

    virtual Setpoints GetSetpoints();
    virtual Protocols::InteractionModel::Status SaveSetpoints(const Setpoints & setpoints,
                                                              SetpointAttributes changedAttributes) = 0;
    void GenerateSetpointEvent(AttributeId attributeId, temperature oldTemp, temperature newTemp) const;
    void NotifyAttributesChanged(const SetpointAttributes & changedAttributes);

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler);

protected:
    ThermostatClusterCore & mCluster;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
