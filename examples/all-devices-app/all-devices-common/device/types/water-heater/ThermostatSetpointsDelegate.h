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

#include <app/clusters/thermostat-server/ThermostatClusterHeatingSetpoints.h>

namespace chip::app {

/*
 * A simple implementation of ThermostatHeatingSetpoints::Delegate.
 */
class ThermostatSetpointsDelegate : public Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate
{
public:
    using temperature = Clusters::Thermostat::temperature;

    ThermostatSetpointsDelegate(EndpointId endpoint = kInvalidEndpointId)
    {}

    Protocols::InteractionModel::Status GetOccupiedHeatingSetpoint(temperature & occupiedHeatingSetpoint) const override;
    Protocols::InteractionModel::Status SetOccupiedHeatingSetpoint(temperature occupiedHeatingSetpoint, bool & changed) override;

private:
    // Mandatory attributes
    temperature mOccupiedHeatingSetpoint = 2000;
};

} // namespace chip::app
