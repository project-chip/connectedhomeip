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
#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <device/api/SingleEndpoint.h>

namespace chip::app {

class Thermostat : public SingleEndpoint
{
public:
    using ThermostatClusterType = Clusters::Thermostat::ThermostatCluster<
        Clusters::Thermostat::Delegate, Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate,
        Clusters::Thermostat::ThermostatCoolingSetpoints::Delegate, Clusters::Thermostat::ThermostatAutoSetpoints::Delegate,
        Clusters::Thermostat::ThermostatPresets::Delegate, Clusters::Thermostat::ThermostatHold::Delegate>;

    struct Context
    {
        TimerDelegate & timerDelegate;
        BitFlags<Clusters::Thermostat::Feature> features = {
            Clusters::Thermostat::Feature::kHeating,
            Clusters::Thermostat::Feature::kCooling,
            Clusters::Thermostat::Feature::kAutoMode,
            Clusters::Thermostat::Feature::kPresets,
        };
        Clusters::Thermostat::OptionalAttributes optionalAttributes = {};
    };

    // Presets and hold delegates are required by this device's expanded cluster composition.
    // Callers of the previous setpoint-only constructor must now supply both delegates.
    Thermostat(const Context & context, Clusters::IdentifyDelegate & identifyDelegate,
               Clusters::Thermostat::Delegate & thermostatDelegate,
               Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate & heatingDelegate,
               Clusters::Thermostat::ThermostatCoolingSetpoints::Delegate & coolingDelegate,
               Clusters::Thermostat::ThermostatAutoSetpoints::Delegate & autoDelegate,
               Clusters::Thermostat::ThermostatPresets::Delegate & presetsDelegate,
               Clusters::Thermostat::ThermostatHold::Delegate & holdDelegate);
    ~Thermostat() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    const BitFlags<Clusters::Thermostat::Feature> & Features() const { return mContext.features; }
    bool HasThermostatCluster() const { return mThermostatCluster.IsConstructed(); }
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    ThermostatClusterType & ThermostatCluster() { return mThermostatCluster.Cluster(); }

protected:
    /// Called before the endpoint is registered, within the registration transaction.
    virtual CHIP_ERROR RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    /// Called after the endpoint is removed, including on partial registration failure.
    /// Overrides must tolerate clusters that were not constructed or registered.
    virtual void UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider) {}

private:
    const Context mContext;
    Clusters::IdentifyDelegate & mIdentifyDelegate;
    Clusters::Thermostat::Delegate & mThermostatDelegate;
    Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate & mHeatingDelegate;
    Clusters::Thermostat::ThermostatCoolingSetpoints::Delegate & mCoolingDelegate;
    Clusters::Thermostat::ThermostatAutoSetpoints::Delegate & mAutoDelegate;
    Clusters::Thermostat::ThermostatPresets::Delegate & mPresetsDelegate;
    Clusters::Thermostat::ThermostatHold::Delegate & mHoldDelegate;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<ThermostatClusterType> mThermostatCluster;
};

} // namespace chip::app
