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

#include <app/clusters/thermostat-server/ThermostatClusterCoolingSetpoints.h>
#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/clusters/thermostat-server/ThermostatDelegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatCoolingSetpointsDelegate : public ThermostatCoolingSetpoints::Delegate
{
public:
    ThermostatCoolingSetpointsDelegate() {}

        virtual Protocols::InteractionModel::Status GetOccupiedCoolingSetpoint(temperature & occupiedCoolingSetpoint) const;
        virtual Protocols::InteractionModel::Status SetOccupiedCoolingSetpoint(temperature occupiedCoolingSetpoint,
                                                                            bool & changed);

        virtual Protocols::InteractionModel::Status GetAbsMinCoolSetpointLimit(temperature & absMinCoolSetpointLimit) const;
        virtual Protocols::InteractionModel::Status GetAbsMaxCoolSetpointLimit(temperature & absMaxCoolSetpointLimit) const;

        virtual Protocols::InteractionModel::Status GetMinCoolSetpointLimit(temperature & minCoolSetpointLimit) const;
        virtual Protocols::InteractionModel::Status SetMinCoolSetpointLimit(temperature minCoolSetpointLimit,
                                                                            bool & changed);

        virtual Protocols::InteractionModel::Status GetMaxCoolSetpointLimit(temperature & maxCoolSetpointLimit) const;
        virtual Protocols::InteractionModel::Status SetMaxCoolSetpointLimit(temperature maxCoolSetpointLimit,
                                                                            bool & changed);

        virtual Protocols::InteractionModel::Status GetUnoccupiedCoolingSetpoint(temperature & unoccupiedCoolingSetpoint) const;
        virtual Protocols::InteractionModel::Status SetUnoccupiedCoolingSetpoint(temperature unoccupiedCoolingSetpoint,
                                                                                bool & changed);
private:
        temperature mOccupiedCoolingSetpoint = 2200;
        temperature mAbsMinCoolSetpointLimit = 1800;
        temperature mAbsMaxCoolSetpointLimit = 2800;
        temperature mMinCoolSetpointLimit = 1800;
        temperature mMaxCoolSetpointLimit = 2800;
        temperature mUnoccupiedCoolingSetpoint = 2200;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
