/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <ElectricalEnergyMeasurementDelegateImpl.h>
#include <ElectricalPowerMeasurementDelegateImpl.h>
#include <PowerTopologyDelegateImpl.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/support/TimerDelegate.h>
#include <platform/DefaultTimerDelegate.h>

namespace chip {
namespace app {
namespace Clusters {

/** @brief Manages the Electrical Power Measurement (EPM), Electrical Energy Measurement (EEM), and
 *  Power Topology (PT) clusters for energy-management example apps.
 *  Holds all three delegates and their cluster instances.
 */
class ElectricalSensorManager
{
public:
    ElectricalSensorManager()                                            = default;
    ~ElectricalSensorManager()                                           = default;
    ElectricalSensorManager(const ElectricalSensorManager &)             = delete;
    ElectricalSensorManager & operator=(const ElectricalSensorManager &) = delete;

    // --- Configuration structs for the clusters to simplify initialization ---
    struct EpmConfig
    {
        BitMask<ElectricalPowerMeasurement::Feature> features;
        BitMask<ElectricalPowerMeasurement::OptionalAttributes> optionalAttributes;
    };

    struct EemConfig
    {
        BitMask<ElectricalEnergyMeasurement::Feature> features;
        ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster::OptionalAttributesSet optionalAttributes;
        const ElectricalEnergyMeasurement::Structs::MeasurementAccuracyStruct::Type & accuracyStruct;
    };

    struct PtConfig
    {
        BitMask<PowerTopology::Feature> features;
    };

    CHIP_ERROR Init(EndpointId endpointId, const EpmConfig & epmConfig, const EemConfig & eemConfig, const PtConfig & ptConfig);
    void Shutdown();

    // --- Setters forwarded to the EEM delegate ---
    void SetCumulativeEnergyImported(int64_t value) { mEEMDelegate.SetCumulativeEnergyImported(value); }
    void SetCumulativeEnergyExported(int64_t value) { mEEMDelegate.SetCumulativeEnergyExported(value); }
    void SetPeriodicEnergyImported(int64_t value) { mEEMDelegate.SetPeriodicEnergyImported(value); }
    void SetPeriodicEnergyExported(int64_t value) { mEEMDelegate.SetPeriodicEnergyExported(value); }
    void GenerateEEMReport();

    // --- Power reading API (updates EPM delegate) ---
    CHIP_ERROR SendPowerReading(int64_t aActivePower_mW, int64_t aVoltage_mV, int64_t aActiveCurrent_mA);

    // --- Cluster / delegate accessors ---
    ElectricalEnergyMeasurement::ElectricalEnergyMeasurementDelegate * GetEEMDelegate() { return &mEEMDelegate; }
    ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster * GetEEMCluster();
    ElectricalPowerMeasurement::ElectricalPowerMeasurementDelegate * GetEPMDelegate() { return mEPMDelegate.get(); }
    ElectricalPowerMeasurement::ElectricalPowerMeasurementInstance * GetEPMInstance() { return mEPMInstance.get(); }
    PowerTopology::PowerTopologyDelegate * GetPTDelegate() { return mPTDelegate.get(); }
    PowerTopology::PowerTopologyInstance * GetPTInstance() { return mPTInstance.get(); }

private:
    // TODO: Have a getter on the Server's TimerDelegate to use here instead of the DefaultTimerDelegate
    DefaultTimerDelegate mTimerDelegate;
    ElectricalEnergyMeasurement::ElectricalEnergyMeasurementDelegate mEEMDelegate;

    // EPM
    std::unique_ptr<ElectricalPowerMeasurement::ElectricalPowerMeasurementDelegate> mEPMDelegate;
    std::unique_ptr<ElectricalPowerMeasurement::ElectricalPowerMeasurementInstance> mEPMInstance;

    // EEM (direct cluster registration, no CodegenIntegration)
    std::unique_ptr<RegisteredServerCluster<ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster>> mEEMCluster;

    // Power Topology
    std::unique_ptr<PowerTopology::PowerTopologyDelegate> mPTDelegate;
    std::unique_ptr<PowerTopology::PowerTopologyInstance> mPTInstance;
};

} // namespace Clusters
} // namespace app
} // namespace chip
