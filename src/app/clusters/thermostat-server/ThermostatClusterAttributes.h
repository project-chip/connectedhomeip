/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <protocols/interaction_model/Constants.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

struct OptionalAttributes
{
    bool AbsMinHeatSetpointLimit = false;
    bool AbsMaxHeatSetpointLimit = false;
    bool AbsMinCoolSetpointLimit = false;
    bool AbsMaxCoolSetpointLimit = false;

    bool LocalTemperatureCalibration = false;

    bool MinHeatSetpointLimit = false;
    bool MaxHeatSetpointLimit = false;
    bool MinCoolSetpointLimit = false;
    bool MaxCoolSetpointLimit = false;

    bool RemoteSensing                   = false;
    bool ThermostatRunningMode           = false;
    bool TemperatureSetpointHold         = false;
    bool TemperatureSetpointHoldDuration = false;
    bool ThermostatRunningState          = false;

    bool SetpointChangeSource            = false;
    bool SetpointChangeAmount            = false;
    bool SetpointChangeSourceTimestamp   = false;

    bool SetpointHoldExpiryTimestamp = false;
    bool OutdoorTemperature          = false;

    OptionalAttributes() = default;
};

CHIP_ERROR AppendOptionalAttributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder,
                                    Span<const AttributeListBuilder::OptionalAttributeEntry> entries);

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
