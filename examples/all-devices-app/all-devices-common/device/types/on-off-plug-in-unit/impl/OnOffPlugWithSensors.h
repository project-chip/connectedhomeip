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

#include <device/types/electrical-sensor/impl/SimulatedElectricalSensor.h>
#include <device/types/on-off-plug-in-unit/OnOffPlugInUnit.h>

namespace chip::app {

class OnOffPlugWithSensors : public OnOffPlugInUnit
{
public:
    OnOffPlugWithSensors(const Context & context, TestEventTriggerDelegate & testEventTriggerDelegate);
    ~OnOffPlugWithSensors() override = default;

    Clusters::ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster & ElectricalEnergyMeasurementCluster()
    {
        return mElectricalSensor.ElectricalEnergyMeasurementCluster();
    }
    Clusters::ElectricalPowerMeasurement::ElectricalPowerMeasurementCluster & ElectricalPowerMeasurementCluster()
    {
        return mElectricalSensor.ElectricalPowerMeasurementCluster();
    }
    Clusters::PowerTopology::PowerTopologyCluster & PowerTopologyCluster() { return mElectricalSensor.PowerTopologyCluster(); }

protected:
    CHIP_ERROR RegisterAdditionalClusters(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    void UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider) override;

private:
    SimulatedElectricalSensor mElectricalSensor;
};

} // namespace chip::app
