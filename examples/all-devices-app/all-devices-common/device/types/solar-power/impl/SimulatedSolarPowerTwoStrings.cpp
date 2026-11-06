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

#include <device/types/solar-power/impl/SimulatedSolarPowerTwoStrings.h>

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

SimulatedSolarPowerTwoStrings::SimulatedSolarPowerTwoStrings(const Context & context) :
    SimulatedSolarPower(context, {}),
    mString1(context.timerDelegate, context.testEventTriggerDelegate, context.fabricTable, Span(SolarPowerTags::kString1Tag)),
    mString2(context.timerDelegate, context.testEventTriggerDelegate, context.fabricTable, Span(SolarPowerTags::kString2Tag))
{}

PanelElectricalSensor & SimulatedSolarPowerTwoStrings::GetStringSensor(size_t index)
{
    PanelElectricalSensor * sensors[] = { &mString1, &mString2 };
    VerifyOrDie(index < kStringCount);
    return *sensors[index];
}

CHIP_ERROR SimulatedSolarPowerTwoStrings::RegisterAdditionalComposedEndpoints(EndpointIdAllocator & allocator,
                                                                              CodeDrivenDataModelProvider & provider,
                                                                              EndpointId parent)
{
    ReturnErrorOnFailure(SimulatedSolarPower::RegisterAdditionalComposedEndpoints(allocator, provider, parent));

    ReturnErrorOnFailure(
        mString1.Register(allocator.Allocate(), provider, ChildComposition(parent, Span(SolarPowerTags::kString1Tag))));
    ReturnErrorOnFailure(
        mString2.Register(allocator.Allocate(), provider, ChildComposition(parent, Span(SolarPowerTags::kString2Tag))));
    return CHIP_NO_ERROR;
}

void SimulatedSolarPowerTwoStrings::UnregisterAdditionalComposedEndpoints(CodeDrivenDataModelProvider & provider)
{
    if (mString2.GetEndpointId() != kInvalidEndpointId)
    {
        mString2.Unregister(provider);
    }
    if (mString1.GetEndpointId() != kInvalidEndpointId)
    {
        mString1.Unregister(provider);
    }
    SimulatedSolarPower::UnregisterAdditionalComposedEndpoints(provider);
}

} // namespace chip::app
