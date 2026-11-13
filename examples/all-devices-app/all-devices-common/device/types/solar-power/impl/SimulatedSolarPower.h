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

#include <array>
#include <memory>

namespace chip::app {

/// Simulated Solar Power with configurable panel, string, or phase measurements.
class SimulatedSolarPower : public SolarPower
{
public:
    using Context = SolarPower::Context;

    struct PanelConfig
    {
        Span<const EndpointComposition::SemanticTag> tags;
        SimulatedElectricalSensor::Role role = SimulatedElectricalSensor::Role::kBattery;
    };

    static constexpr size_t kMaxPanelCount = 4;

    struct Config
    {
        SimulatedElectricalSensor::Role outputSensorRole = SimulatedElectricalSensor::Role::kGrid;
        Span<const PanelConfig> panels;
        Span<const EndpointComposition::SemanticTag> tags;
    };

    /// Config supports at most kMaxPanelCount panels. All tag storage must outlive the device;
    /// the panel configuration array is only used during construction.
    explicit SimulatedSolarPower(const Context & context, const Config & config);
    ~SimulatedSolarPower() override = default;

    /// Uses Config::tags when composition does not supply parent tags.
    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;

    size_t GetPanelCount() const { return mPanelCount; }
    PanelElectricalSensor & GetPanelSensor(size_t index);

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
    Span<const EndpointComposition::SemanticTag> mTags;
    std::array<std::unique_ptr<PanelElectricalSensor>, kMaxPanelCount> mPanels;
    const size_t mPanelCount;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    EnergyManagement mEnergyManagement;
    TemperatureSensor mInverterTemperatureSensor;
};

} // namespace chip::app
