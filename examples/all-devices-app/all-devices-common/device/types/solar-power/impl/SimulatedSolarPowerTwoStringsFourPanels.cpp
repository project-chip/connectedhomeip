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

#include <device/types/solar-power/impl/SimulatedSolarPowerTwoStringsFourPanels.h>

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

SimulatedSolarPowerTwoStringsFourPanels::SimulatedSolarPowerTwoStringsFourPanels(const Context & context) :
    SimulatedSolarPower(context, {}), mString1Panel1(context.timerDelegate, context.testEventTriggerDelegate, context.fabricTable,
                                                     Span(SolarPowerTags::kString1Panel1Tags)),
    mString1Panel2(context.timerDelegate, context.testEventTriggerDelegate, context.fabricTable,
                   Span(SolarPowerTags::kString1Panel2Tags)),
    mString2Panel1(context.timerDelegate, context.testEventTriggerDelegate, context.fabricTable,
                   Span(SolarPowerTags::kString2Panel1Tags)),
    mString2Panel2(context.timerDelegate, context.testEventTriggerDelegate, context.fabricTable,
                   Span(SolarPowerTags::kString2Panel2Tags))
{}

PanelElectricalSensor & SimulatedSolarPowerTwoStringsFourPanels::GetPanelSensor(size_t index)
{
    PanelElectricalSensor * sensors[] = { &mString1Panel1, &mString1Panel2, &mString2Panel1, &mString2Panel2 };
    VerifyOrDie(index < kPanelEndpointCount);
    return *sensors[index];
}

CHIP_ERROR SimulatedSolarPowerTwoStringsFourPanels::RegisterAdditionalComposedEndpoints(EndpointIdAllocator & allocator,
                                                                                        CodeDrivenDataModelProvider & provider,
                                                                                        EndpointId parent)
{
    ReturnErrorOnFailure(SimulatedSolarPower::RegisterAdditionalComposedEndpoints(allocator, provider, parent));

    for (uint8_t stringIndex = 1; stringIndex <= kStringCount; stringIndex++)
    {
        for (uint8_t panelIndex = 1; panelIndex <= kPanelsPerString; panelIndex++)
        {
            const size_t flatIndex         = static_cast<size_t>((stringIndex - 1) * kPanelsPerString + (panelIndex - 1));
            PanelElectricalSensor & sensor = GetPanelSensor(flatIndex);
            ReturnErrorOnFailure(
                sensor.Register(allocator.Allocate(), provider,
                                ChildComposition(parent, SolarPowerTags::StringPanelTags(stringIndex, panelIndex))));
        }
    }
    return CHIP_NO_ERROR;
}

void SimulatedSolarPowerTwoStringsFourPanels::UnregisterAdditionalComposedEndpoints(CodeDrivenDataModelProvider & provider)
{
    for (size_t index = 0; index < kPanelEndpointCount; index++)
    {
        PanelElectricalSensor & sensor = GetPanelSensor(index);
        if (sensor.GetEndpointId() != kInvalidEndpointId)
        {
            sensor.Unregister(provider);
        }
    }
    SimulatedSolarPower::UnregisterAdditionalComposedEndpoints(provider);
}

} // namespace chip::app
