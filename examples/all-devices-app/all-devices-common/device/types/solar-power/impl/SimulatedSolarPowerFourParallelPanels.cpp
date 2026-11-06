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

#include <device/types/solar-power/impl/SimulatedSolarPowerFourParallelPanels.h>

#include <device/types/solar-power/SolarPowerTags.h>

namespace chip::app {

namespace {

EndpointComposition ChildComposition(EndpointId parent, Span<const EndpointComposition::SemanticTag> tags)
{
    EndpointComposition composition = EndpointComposition::WithParent(parent);
    composition.tagList             = tags;
    return composition;
}

} // namespace

SimulatedSolarPowerFourParallelPanels::SimulatedSolarPowerFourParallelPanels(const Context & context) :
    SimulatedSolarPower(context, {}), mParallelPanel1(context.timerDelegate, context.testEventTriggerDelegate, context.fabricTable,
                                                      Span(SolarPowerTags::kParallelPanel1Tag)),
    mParallelPanel2(context.timerDelegate, context.testEventTriggerDelegate, context.fabricTable,
                    Span(SolarPowerTags::kParallelPanel2Tag)),
    mParallelPanel3(context.timerDelegate, context.testEventTriggerDelegate, context.fabricTable,
                    Span(SolarPowerTags::kParallelPanel3Tag)),
    mParallelPanel4(context.timerDelegate, context.testEventTriggerDelegate, context.fabricTable,
                    Span(SolarPowerTags::kParallelPanel4Tag))
{}

PanelElectricalSensor & SimulatedSolarPowerFourParallelPanels::GetParallelPanelSensor(size_t index)
{
    PanelElectricalSensor * sensors[] = { &mParallelPanel1, &mParallelPanel2, &mParallelPanel3, &mParallelPanel4 };
    VerifyOrDie(index < kParallelPanelCount);
    return *sensors[index];
}

CHIP_ERROR SimulatedSolarPowerFourParallelPanels::RegisterAdditionalComposedEndpoints(EndpointIdAllocator & allocator,
                                                                                      CodeDrivenDataModelProvider & provider,
                                                                                      EndpointId parent)
{
    ReturnErrorOnFailure(SimulatedSolarPower::RegisterAdditionalComposedEndpoints(allocator, provider, parent));

    for (uint8_t panelIndex = 1; panelIndex <= kParallelPanelCount; panelIndex++)
    {
        PanelElectricalSensor & sensor = GetParallelPanelSensor(panelIndex - 1);
        ReturnErrorOnFailure(sensor.Register(allocator.Allocate(), provider,
                                             ChildComposition(parent, SolarPowerTags::ParallelPanelTag(panelIndex))));
    }
    return CHIP_NO_ERROR;
}

void SimulatedSolarPowerFourParallelPanels::UnregisterAdditionalComposedEndpoints(CodeDrivenDataModelProvider & provider)
{
    for (size_t index = 0; index < kParallelPanelCount; index++)
    {
        PanelElectricalSensor & sensor = GetParallelPanelSensor(index);
        if (sensor.GetEndpointId() != kInvalidEndpointId)
        {
            sensor.Unregister(provider);
        }
    }
    SimulatedSolarPower::UnregisterAdditionalComposedEndpoints(provider);
}

} // namespace chip::app
