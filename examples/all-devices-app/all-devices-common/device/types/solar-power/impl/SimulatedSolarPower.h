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
#include <device/types/device-energy-management/EnergyManagement.h>
#include <device/types/solar-power/SolarPower.h>
#include <device/types/temperature-sensor/TemperatureSensor.h>

namespace chip::app {

/// Simulated single-phase AC Solar Power (Figure 17): Identify, DEM, inverter temperature, grid power source, AC export sensor.
class SimulatedSolarPower : public SolarPower
{
public:
    using Context = SolarPower::Context;

    explicit SimulatedSolarPower(const Context & context, Parameters parameters);
    ~SimulatedSolarPower() override = default;

    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    EnergyManagement & GetEnergyManagement() { return mEnergyManagement; }
    TemperatureSensor & GetInverterTemperatureSensor() { return mInverterTemperatureSensor; }

protected:
    CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) override;

    CHIP_ERROR RegisterAdditionalComposedEndpoints(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                                   EndpointId parent) override;
    void UnregisterAdditionalComposedEndpoints(CodeDrivenDataModelProvider & provider) override;

private:
    TimerDelegate & mTimerDelegate;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    EnergyManagement mEnergyManagement;
    TemperatureSensor mInverterTemperatureSensor;
};

} // namespace chip::app
