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

/// Three-phase AC export with per-phase Electrical Sensor child endpoints (Electrical Measurement namespace tags).
class SimulatedSolarPowerThreePhase : public SimulatedSolarPower
{
public:
    explicit SimulatedSolarPowerThreePhase(const Context & context);
    ~SimulatedSolarPowerThreePhase() override = default;

    SimulatedElectricalSensor & GetPhaseSensor(size_t index);

protected:
    CHIP_ERROR RegisterAdditionalComposedEndpoints(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                                   EndpointId parent) override;
    void UnregisterAdditionalComposedEndpoints(CodeDrivenDataModelProvider & provider) override;

private:
    static constexpr size_t kPhaseCount = 3;

    PanelElectricalSensor mPhaseA;
    PanelElectricalSensor mPhaseB;
    PanelElectricalSensor mPhaseC;
};

} // namespace chip::app
