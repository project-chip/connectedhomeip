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

#include <device/types/solar-power/PanelElectricalSensor.h>
#include <device/types/solar-power/impl/SimulatedSolarPower.h>

namespace chip::app {

/// Figure 18: four parallel panel-group DC measurements in addition to the Figure 17 AC export path.
class SimulatedSolarPowerFourParallelPanels : public SimulatedSolarPower
{
public:
    static constexpr size_t kParallelPanelCount = 4;

    explicit SimulatedSolarPowerFourParallelPanels(const Context & context);
    ~SimulatedSolarPowerFourParallelPanels() override = default;

    PanelElectricalSensor & GetParallelPanelSensor(size_t index);

protected:
    CHIP_ERROR RegisterAdditionalComposedEndpoints(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                                   EndpointId parent) override;
    void UnregisterAdditionalComposedEndpoints(CodeDrivenDataModelProvider & provider) override;

private:
    PanelElectricalSensor mParallelPanel1;
    PanelElectricalSensor mParallelPanel2;
    PanelElectricalSensor mParallelPanel3;
    PanelElectricalSensor mParallelPanel4;
};

} // namespace chip::app
