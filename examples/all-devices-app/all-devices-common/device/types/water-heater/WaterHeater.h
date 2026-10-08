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

#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/clusters/water-heater-management-server/WaterHeaterManagementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <device/api/SingleEndpoint.h>
#include <devices/Types.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class WaterHeater : public SingleEndpoint
{
public:
    using HeatingThermostat = Clusters::Thermostat::ThermostatCluster<Clusters::Thermostat::Delegate,
                                                                      Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate>;

    struct Config
    {
        TimerDelegate & timerDelegate;
        FabricTable & fabricTable;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
        // WaterHeaterManagement cluster
        BitMask<Clusters::WaterHeaterManagement::Feature> whmFeatures;
        Clusters::WaterHeaterManagement::Delegate & waterHeaterManagementDelegate;
        // Thermostat cluster
        BitMask<Clusters::Thermostat::Feature> thermostatFeatures;
        Clusters::Thermostat::OptionalAttributes thermostatOptionalAttributes;
        Clusters::Thermostat::Delegate & thermostatDelegate;
        Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate & heatingDelegate;
        // ModeBase cluster
        Clusters::ModeBase::AppDelegate & waterHeaterModeDelegate;
    };

    explicit WaterHeater(const Config & config);
    ~WaterHeater();

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;

    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::WaterHeaterManagement::WaterHeaterManagementCluster & WaterHeaterManagementCluster();

    HeatingThermostat & ThermostatCluster();

    Clusters::ModeBaseCluster & WaterHeaterModeCluster();

protected:
    virtual CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}

    Config mConfig;
    CodeDrivenDataModelProvider * mProvider = nullptr;

    // Delegates
    TimerDelegate & mTimerDelegate;
    Clusters::WaterHeaterManagement::Delegate & mWhmDelegate;
    Clusters::ModeBase::AppDelegate & mWaterHeaterModeDelegate;
    Clusters::Thermostat::Delegate & mThermostatDelegate;
    Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate & mHeatingDelegate;

private:
    // Clusters
    LazyRegisteredServerCluster<Clusters::WaterHeaterManagement::WaterHeaterManagementCluster> mWaterHeaterManagementCluster;
    LazyRegisteredServerCluster<HeatingThermostat> mThermostatCluster;
    LazyRegisteredServerCluster<Clusters::ModeBaseCluster> mWaterHeaterModeCluster;
};

} // namespace chip::app
