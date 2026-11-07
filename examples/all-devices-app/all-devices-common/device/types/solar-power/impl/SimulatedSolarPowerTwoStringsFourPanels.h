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

/// Figure 19: two strings with two panels each — four per-panel DC Electrical Sensor endpoints.
class SimulatedSolarPowerTwoStringsFourPanels : public SimulatedSolarPower
{
public:
    static constexpr size_t kStringCount        = 2;
    static constexpr size_t kPanelsPerString    = 2;
    static constexpr size_t kPanelEndpointCount = kStringCount * kPanelsPerString;

    explicit SimulatedSolarPowerTwoStringsFourPanels(const Context & context);
    ~SimulatedSolarPowerTwoStringsFourPanels() override = default;

    PanelElectricalSensor & GetPanelSensor(size_t index);

protected:
    CHIP_ERROR RegisterAdditionalComposedEndpoints(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                                   EndpointId parent) override;
    void UnregisterAdditionalComposedEndpoints(CodeDrivenDataModelProvider & provider) override;

private:
    PanelElectricalSensor mString1Panel1;
    PanelElectricalSensor mString1Panel2;
    PanelElectricalSensor mString2Panel1;
    PanelElectricalSensor mString2Panel2;
};

} // namespace chip::app
