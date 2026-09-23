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
#include <device/api/Interface.h>

namespace chip::app {

/// A room air conditioner with cooling and optional child endpoints.
/// Optional clusters and child devices belong to subclasses.
class RoomAirConditioner : public DeviceInterface
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
    };

    explicit RoomAirConditioner(const Context & context);
    ~RoomAirConditioner() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;

    void Unregister(CodeDrivenDataModelProvider & provider) override;

    EndpointId GetEndpointId() const { return mEndpointId; }

    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::OnOffCluster & OnOffCluster() { return mOnOffCluster.Cluster(); }
    CoolingThermostat & ThermostatCluster() { return mThermostatCluster.Cluster(); }

protected:
    /// Called before the endpoint is registered, within the registration transaction.
    virtual CHIP_ERROR RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    /// Called after the endpoint is removed, including on partial registration failure.
    /// Overrides must tolerate clusters that were not constructed or registered.
    virtual void UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider) {}

    /// Called after the parent endpoint is registered, within the same registration transaction.
    /// Subclasses register owned children using allocator and GetEndpointId() as their parent.
    /// Semantic tags and their backing storage belong to the subclass.
    virtual CHIP_ERROR RegisterAdditionalEndpoints(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    /// Called before the parent endpoint is removed, also after partial registration failure.
    /// Subclasses must remove only registered children, in reverse registration order.
    virtual void UnregisterAdditionalEndpoints(CodeDrivenDataModelProvider & provider) {}

private:
    EndpointId mEndpointId = kInvalidEndpointId;
    TimerDelegate & mTimerDelegate;
    Clusters::IdentifyDelegate & mIdentifyDelegate;
    Clusters::OnOffDelegate & mOnOffDelegate;
    Clusters::Thermostat::Delegate & mThermostatDelegate;
    Clusters::Thermostat::ThermostatCoolingSetpoints::Delegate & mCoolingDelegate;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::OnOffCluster> mOnOffCluster;
    LazyRegisteredServerCluster<CoolingThermostat> mThermostatCluster;
};

} // namespace chip::app
