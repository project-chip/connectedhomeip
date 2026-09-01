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

#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementCluster.h>
#include <app/clusters/electrical-power-measurement-server/ElectricalPowerMeasurementCluster.h>
#include <app/clusters/power-topology-server/PowerTopologyCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class ElectricalSensor : public SingleEndpoint
{
protected:
    using ElectricalEnergyMeasurementClusterT = Clusters::ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster;
    using ElectricalPowerMeasurementClusterT  = Clusters::ElectricalPowerMeasurement::ElectricalPowerMeasurementCluster;
    using PowerTopologyClusterT               = Clusters::PowerTopology::PowerTopologyCluster;

public:
    struct Config
    {
        Clusters::ElectricalEnergyMeasurement::Delegate & electricalEnergyDelegate;
        BitMask<Clusters::ElectricalEnergyMeasurement::Feature> electricalEnergyMeasurementFeatureFlags;
        ElectricalEnergyMeasurementClusterT::OptionalAttributesSet electricalEnergyMeasurementOptionalAttributes;
        Clusters::ElectricalEnergyMeasurement::Structs::MeasurementAccuracyStruct::Type electricalEnergyMeasurementAccuracyStruct;

        Clusters::ElectricalPowerMeasurement::Delegate & electricalPowerDelegate;
        BitMask<Clusters::ElectricalPowerMeasurement::Feature> electricalPowerMeasurementFeatureFlags;
        ElectricalPowerMeasurementClusterT::OptionalAttributesSet electricalPowerMeasurementOptionalAttributes;

        Clusters::PowerTopology::Delegate & powerTopologyDelegate;
        BitMask<Clusters::PowerTopology::Feature> powerTopologyFeatures;

        TimerDelegate & timerDelegate;
    };
    ElectricalSensor(const Config & config);
    ~ElectricalSensor() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Accessors for subclasses/implementations to interact with clusters
    ElectricalEnergyMeasurementClusterT & ElectricalEnergyMeasurementCluster();
    ElectricalPowerMeasurementClusterT & ElectricalPowerMeasurementCluster();
    PowerTopologyClusterT & PowerTopologyCluster();

protected:
    Config mConfig;

    LazyRegisteredServerCluster<ElectricalEnergyMeasurementClusterT> mElectricalEnergyMeasurementCluster;
    LazyRegisteredServerCluster<ElectricalPowerMeasurementClusterT> mElectricalPowerMeasurementCluster;
    LazyRegisteredServerCluster<PowerTopologyClusterT> mPowerTopologyCluster;
};

} // namespace chip::app
