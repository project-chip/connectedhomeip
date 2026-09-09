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

#include <app/clusters/thermostat-server/ThermostatClusterCoolingSetpoints.h>
#include <app/clusters/thermostat-server/ThermostatClusterHeatingSetpoints.h>
#include <app/clusters/thermostat-server/ThermostatClusterSetpoints.h>
#include <app/clusters/thermostat-server/ThermostatDelegate.h>

#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

/*
 * A simple implementation of ThermostatCoolingSetpoints::Delegate,
 * ThermostatHeatingSetpoints::Delegate, and ThermostatAutoSetpoints::Delegate.
 * It reports and persists the state of the thermostat's setpoints attributes.
 *
 * It also demonstrates that a single class can implement multiple related thermostat delegates.
 */
class ThermostatSetpointsDelegate : public ThermostatCoolingSetpoints::Delegate,
                                    public ThermostatHeatingSetpoints::Delegate,
                                    public ThermostatAutoSetpoints::Delegate
{
public:
    ThermostatSetpointsDelegate(EndpointId endpoint, AttributePersistenceProvider * provider = nullptr) :
        mEndpointId(endpoint), mProvider(provider)
    {}

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType type) override;

    Protocols::InteractionModel::Status GetMinDeadband(temperature & minDeadband) const override;

    Protocols::InteractionModel::Status GetOccupiedCoolingSetpoint(temperature & occupiedCoolingSetpoint) const override;
    Protocols::InteractionModel::Status SetOccupiedCoolingSetpoint(temperature occupiedCoolingSetpoint, bool & changed) override;

    Protocols::InteractionModel::Status GetUnoccupiedCoolingSetpoint(temperature & unoccupiedCoolingSetpoint) const override;
    Protocols::InteractionModel::Status SetUnoccupiedCoolingSetpoint(temperature unoccupiedCoolingSetpoint,
                                                                     bool & changed) override;

    Protocols::InteractionModel::Status GetAbsMinCoolSetpointLimit(temperature & absMinCoolSetpointLimit) const override;
    Protocols::InteractionModel::Status GetAbsMaxCoolSetpointLimit(temperature & absMaxCoolSetpointLimit) const override;

    Protocols::InteractionModel::Status GetMinCoolSetpointLimit(temperature & minCoolSetpointLimit) const override;
    Protocols::InteractionModel::Status SetMinCoolSetpointLimit(temperature minCoolSetpointLimit, bool & changed) override;

    Protocols::InteractionModel::Status GetMaxCoolSetpointLimit(temperature & maxCoolSetpointLimit) const override;
    Protocols::InteractionModel::Status SetMaxCoolSetpointLimit(temperature maxCoolSetpointLimit, bool & changed) override;

    Protocols::InteractionModel::Status GetOccupiedHeatingSetpoint(temperature & occupiedHeatingSetpoint) const override;
    Protocols::InteractionModel::Status SetOccupiedHeatingSetpoint(temperature occupiedHeatingSetpoint, bool & changed) override;

    Protocols::InteractionModel::Status GetUnoccupiedHeatingSetpoint(temperature & unoccupiedHeatingSetpoint) const override;
    Protocols::InteractionModel::Status SetUnoccupiedHeatingSetpoint(temperature unoccupiedHeatingSetpoint,
                                                                     bool & changed) override;

    Protocols::InteractionModel::Status GetAbsMinHeatSetpointLimit(temperature & absMinHeatSetpointLimit) const override;
    Protocols::InteractionModel::Status GetAbsMaxHeatSetpointLimit(temperature & absMaxHeatSetpointLimit) const override;

    Protocols::InteractionModel::Status GetMinHeatSetpointLimit(temperature & minHeatSetpointLimit) const override;
    Protocols::InteractionModel::Status SetMinHeatSetpointLimit(temperature minHeatSetpointLimit, bool & changed) override;

    Protocols::InteractionModel::Status GetMaxHeatSetpointLimit(temperature & maxHeatSetpointLimit) const override;
    Protocols::InteractionModel::Status SetMaxHeatSetpointLimit(temperature maxHeatSetpointLimit, bool & changed) override;

private:
    EndpointId mEndpointId;
    AttributePersistenceProvider * mProvider = nullptr;
    bool mStarted                            = false;

    temperature mOccupiedCoolingSetpoint;
    temperature mUnoccupiedCoolingSetpoint;
    temperature mAbsMinCoolSetpointLimit;
    temperature mAbsMaxCoolSetpointLimit;
    temperature mMinCoolSetpointLimit;
    temperature mMaxCoolSetpointLimit;

    temperature mOccupiedHeatingSetpoint;
    temperature mUnoccupiedHeatingSetpoint;
    temperature mAbsMinHeatSetpointLimit;
    temperature mAbsMaxHeatSetpointLimit;
    temperature mMinHeatSetpointLimit;
    temperature mMaxHeatSetpointLimit;

    Protocols::InteractionModel::Status SaveSetpoint(AttributeId attributeId, temperature value);
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
