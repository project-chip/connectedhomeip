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

#include <device/types/solar-power/impl/SimulatedSolarPower.h>

#include <device/types/solar-power/SolarPowerTags.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

namespace {

using DemFeature = Clusters::DeviceEnergyManagement::Feature;

EnergyManagement::Config SolarPowerEnergyManagementConfig()
{
    return EnergyManagement::Config{
        .featureFlags   = BitMask<DemFeature>(DemFeature::kPowerAdjustment),
        .esaType        = Clusters::DeviceEnergyManagement::ESATypeEnum::kSolarPV,
        .esaCanGenerate = true,
    };
}

Clusters::TemperatureMeasurementCluster::StartupConfiguration InverterTemperatureConfig()
{
    return {
        .minMeasuredValue = DataModel::MakeNullable(static_cast<int16_t>(-2000)),
        .maxMeasuredValue = DataModel::MakeNullable(static_cast<int16_t>(8500)),
    };
}

EndpointComposition ChildComposition(EndpointId parent, Span<const EndpointComposition::SemanticTag> tags)
{
    EndpointComposition composition = EndpointComposition::WithParent(parent);
    composition.tagList             = tags;
    return composition;
}

} // namespace

SimulatedSolarPower::SimulatedSolarPower(const Context & context, Parameters parameters) :
    SolarPower(context, parameters), mTimerDelegate(context.timerDelegate),
    mEnergyManagement(context.timerDelegate, SolarPowerEnergyManagementConfig()),
    mInverterTemperatureSensor(context.timerDelegate, InverterTemperatureConfig())
{}

CHIP_ERROR SimulatedSolarPower::RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mTimerDelegate));
    return provider.AddCluster(mIdentifyCluster.Registration());
}

void SimulatedSolarPower::UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider)
{
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

CHIP_ERROR SimulatedSolarPower::RegisterAdditionalComposedEndpoints(EndpointIdAllocator & allocator,
                                                                    CodeDrivenDataModelProvider & provider, EndpointId parent)
{
    ReturnErrorOnFailure(mEnergyManagement.Register(allocator.Allocate(), provider, EndpointComposition::WithParent(parent)));
    ReturnErrorOnFailure(mInverterTemperatureSensor.Register(
        allocator.Allocate(), provider, ChildComposition(parent, Span(SolarPowerTags::kInverterTemperatureTag))));
    return CHIP_NO_ERROR;
}

void SimulatedSolarPower::UnregisterAdditionalComposedEndpoints(CodeDrivenDataModelProvider & provider)
{
    if (mInverterTemperatureSensor.GetEndpointId() != kInvalidEndpointId)
    {
        mInverterTemperatureSensor.Unregister(provider);
    }
    if (mEnergyManagement.GetEndpointId() != kInvalidEndpointId)
    {
        mEnergyManagement.Unregister(provider);
    }
}

} // namespace chip::app
