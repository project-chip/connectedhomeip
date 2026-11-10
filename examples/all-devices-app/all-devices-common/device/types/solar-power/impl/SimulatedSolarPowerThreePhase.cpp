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

#include <device/types/solar-power/impl/SimulatedSolarPowerThreePhase.h>

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

SimulatedSolarPowerThreePhase::SimulatedSolarPowerThreePhase(const Context & context) :
    SimulatedSolarPower(context, Parameters{ .outputSensorRole = SimulatedElectricalSensor::Role::kAcPolyphaseOutput }),
    mPhaseA(context.timerDelegate, context.testEventTriggerDelegate, context.fabricTable, Span(SolarPowerTags::kPhaseATag),
            SimulatedElectricalSensor::Role::kGrid),
    mPhaseB(context.timerDelegate, context.testEventTriggerDelegate, context.fabricTable, Span(SolarPowerTags::kPhaseBTag),
            SimulatedElectricalSensor::Role::kGrid),
    mPhaseC(context.timerDelegate, context.testEventTriggerDelegate, context.fabricTable, Span(SolarPowerTags::kPhaseCTag),
            SimulatedElectricalSensor::Role::kGrid)
{}

SimulatedElectricalSensor & SimulatedSolarPowerThreePhase::GetPhaseSensor(size_t index)
{
    PanelElectricalSensor * sensors[] = { &mPhaseA, &mPhaseB, &mPhaseC };
    VerifyOrDie(index < kPhaseCount);
    return *sensors[index];
}

CHIP_ERROR SimulatedSolarPowerThreePhase::RegisterAdditionalComposedEndpoints(EndpointIdAllocator & allocator,
                                                                              CodeDrivenDataModelProvider & provider,
                                                                              EndpointId parent)
{
    ReturnErrorOnFailure(SimulatedSolarPower::RegisterAdditionalComposedEndpoints(allocator, provider, parent));

    ReturnErrorOnFailure(
        mPhaseA.Register(allocator.Allocate(), provider, ChildComposition(parent, Span(SolarPowerTags::kPhaseATag))));
    ReturnErrorOnFailure(
        mPhaseB.Register(allocator.Allocate(), provider, ChildComposition(parent, Span(SolarPowerTags::kPhaseBTag))));
    ReturnErrorOnFailure(
        mPhaseC.Register(allocator.Allocate(), provider, ChildComposition(parent, Span(SolarPowerTags::kPhaseCTag))));
    return CHIP_NO_ERROR;
}

void SimulatedSolarPowerThreePhase::UnregisterAdditionalComposedEndpoints(CodeDrivenDataModelProvider & provider)
{
    for (size_t index = 0; index < kPhaseCount; index++)
    {
        SimulatedElectricalSensor & sensor = GetPhaseSensor(index);
        if (sensor.GetEndpointId() != kInvalidEndpointId)
        {
            sensor.Unregister(provider);
        }
    }
    SimulatedSolarPower::UnregisterAdditionalComposedEndpoints(provider);
}

} // namespace chip::app
