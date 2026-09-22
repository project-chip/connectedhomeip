/*
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

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/clusters/thermostat-user-interface-configuration-server/ThermostatUserInterfaceConfigurationCluster.h>
#include <device/api/SingleEndpoint.h>

namespace chip::app {

/// A single-endpoint room air conditioner with cooling and local UI configuration.
class RoomAirConditioner : public SingleEndpoint
{
public:
    using CoolingThermostat = Clusters::Thermostat::ThermostatCluster<Clusters::Thermostat::Delegate,
                                                                      Clusters::Thermostat::ThermostatCoolingSetpoints::Delegate>;

    struct Context
    {
        TimerDelegate & timerDelegate;
        Clusters::IdentifyDelegate & identifyDelegate;
        Clusters::OnOffDelegate & onOffDelegate;
        Clusters::Thermostat::Delegate & thermostatDelegate;
        Clusters::Thermostat::ThermostatCoolingSetpoints::Delegate & coolingDelegate;
        Clusters::ThermostatUserInterfaceConfiguration::Delegate & userInterfaceDelegate;
    };

    explicit RoomAirConditioner(const Context & context);
    ~RoomAirConditioner() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::OnOffCluster & OnOffCluster() { return mOnOffCluster.Cluster(); }
    CoolingThermostat & ThermostatCluster() { return mThermostatCluster.Cluster(); }
    Clusters::ThermostatUserInterfaceConfigurationCluster & UserInterfaceCluster() { return mUserInterfaceCluster.Cluster(); }

private:
    TimerDelegate & mTimerDelegate;
    Clusters::IdentifyDelegate & mIdentifyDelegate;
    Clusters::OnOffDelegate & mOnOffDelegate;
    Clusters::Thermostat::Delegate & mThermostatDelegate;
    Clusters::Thermostat::ThermostatCoolingSetpoints::Delegate & mCoolingDelegate;
    Clusters::ThermostatUserInterfaceConfiguration::Delegate & mUserInterfaceDelegate;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::OnOffCluster> mOnOffCluster;
    LazyRegisteredServerCluster<CoolingThermostat> mThermostatCluster;
    LazyRegisteredServerCluster<Clusters::ThermostatUserInterfaceConfigurationCluster> mUserInterfaceCluster;
};

} // namespace chip::app
