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

#include <app/clusters/groups-server/GroupsCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/clusters/thermostat-user-interface-configuration-server/ThermostatUserInterfaceConfigurationCluster.h>
#include <device/api/SingleEndpoint.h>

namespace chip::app {

class Thermostat : public SingleEndpoint
{
public:
    using ThermostatClusterType =
        Clusters::Thermostat::ThermostatCluster<Clusters::Thermostat::Delegate,
                                                Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate,
                                                Clusters::Thermostat::ThermostatCoolingSetpoints::Delegate>;

    struct Context
    {
        Credentials::GroupDataProvider & groupDataProvider;
        FabricTable & fabricTable;
        TimerDelegate & timerDelegate;
    };

    Thermostat(const Context & context, Clusters::IdentifyDelegate & identifyDelegate,
               Clusters::Thermostat::Delegate & thermostatDelegate,
               Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate & heatingDelegate,
               Clusters::Thermostat::ThermostatCoolingSetpoints::Delegate & coolingDelegate,
               Clusters::ThermostatUserInterfaceConfiguration::Delegate & userInterfaceDelegate);
    ~Thermostat() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::GroupsCluster & GroupsCluster() { return mGroupsCluster.Cluster(); }
    ThermostatClusterType & ThermostatCluster() { return mThermostatCluster.Cluster(); }
    Clusters::ThermostatUserInterfaceConfigurationCluster & ThermostatUserInterfaceConfigurationCluster()
    {
        return mUserInterfaceCluster.Cluster();
    }

private:
    const Context mContext;
    Clusters::IdentifyDelegate & mIdentifyDelegate;
    Clusters::Thermostat::Delegate & mThermostatDelegate;
    Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate & mHeatingDelegate;
    Clusters::Thermostat::ThermostatCoolingSetpoints::Delegate & mCoolingDelegate;
    Clusters::ThermostatUserInterfaceConfiguration::Delegate & mUserInterfaceDelegate;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::GroupsCluster> mGroupsCluster;
    LazyRegisteredServerCluster<ThermostatClusterType> mThermostatCluster;
    LazyRegisteredServerCluster<Clusters::ThermostatUserInterfaceConfigurationCluster> mUserInterfaceCluster;
};

} // namespace chip::app
