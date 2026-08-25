/*
 *    Copyright (c) 2024-2026 Project CHIP Authors
 *    All rights reserved.
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

#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>

#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/clusters/thermostat-server/ThermostatClusterHold.h>
#include <app/clusters/thermostat-server/ThermostatDelegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatHoldDelegate : public ThermostatHold::Delegate
{
public:
    ThermostatHoldDelegate(EndpointId endpoint, AttributePersistenceProvider * provider = nullptr) :
        mEndpointId(endpoint), mProvider(provider)
    {}

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    TemperatureSetpointHoldEnum GetTemperatureSetpointHold() const override;
    Protocols::InteractionModel::Status SetTemperatureSetpointHold(TemperatureSetpointHoldEnum hold, bool & changed) override;

    DataModel::Nullable<uint16_t> GetTemperatureSetpointHoldDuration() const override;
    Protocols::InteractionModel::Status SetTemperatureSetpointHoldDuration(DataModel::Nullable<uint16_t> duration,
                                                                           bool & changed) override;

    DataModel::Nullable<uint32_t> GetSetpointHoldExpiryTimestamp() const override;
    Protocols::InteractionModel::Status SetSetpointHoldExpiryTimestamp(DataModel::Nullable<uint32_t> timestamp,
                                                                       bool & changed) override;


private:
    EndpointId mEndpointId;
    AttributePersistenceProvider * mProvider;

    TemperatureSetpointHoldEnum mTemperatureSetpointHold = TemperatureSetpointHoldEnum::kSetpointHoldOff;
    DataModel::Nullable<uint16_t> mTemperatureSetpointHoldDuration;
    DataModel::Nullable<uint32_t> mSetpointHoldExpiryTimestamp;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
