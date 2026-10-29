/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <device/types/battery-storage/BatteryStorage.h>

#include <devices/Types.h>

namespace chip::app {

namespace {

using DemFeature = Clusters::DeviceEnergyManagement::Feature;
using EsaType    = Clusters::DeviceEnergyManagement::ESATypeEnum;

// Battery Storage rev 2 component tags use the Power Source namespace (Grid / Battery).
const EndpointComposition::SemanticTag kGridComponentTag[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kPowerSourceId, .tag = 0x01 },
};
const EndpointComposition::SemanticTag kBatteryComponentTag[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kPowerSourceId, .tag = 0x03 },
};

EndpointComposition ChildComposition(EndpointId parent, Span<const EndpointComposition::SemanticTag> tags)
{
    EndpointComposition composition = EndpointComposition::WithParent(parent);
    composition.tagList             = tags;
    return composition;
}

EnergyManagement::Config BatteryStorageEnergyManagementConfig()
{
    return EnergyManagement::Config{
        .featureFlags   = BitMask<DemFeature>(DemFeature::kPowerAdjustment),
        .esaType        = EsaType::kBatteryStorage,
        .esaCanGenerate = true,
    };
}

} // namespace

BatteryStorage::BatteryStorage(const Context & context) :
    DeviceInterface(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kBatteryStorage, 1)),
    mEnergyManagement(context.timerDelegate, BatteryStorageEnergyManagementConfig()),
    mGridElectricalSensor(context.timerDelegate, context.testEventTriggerDelegate, {}, SimulatedElectricalSensor::Role::kGrid),
    mBatteryElectricalSensor(context.timerDelegate, context.testEventTriggerDelegate, {},
                             SimulatedElectricalSensor::Role::kBattery),
    mGridPowerSource("Battery storage grid connection"_span, Clusters::PowerSource::WiredCurrentTypeEnum::kAc, 0),
    mBatteryPowerSource("Battery storage pack"_span, Clusters::PowerSource::BatReplaceabilityEnum::kNotReplaceable,
                        context.timerDelegate, 1, BatteryPowerSource::ChargingCapability::kRechargeable)
{}

CHIP_ERROR BatteryStorage::Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                    EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    const EndpointId endpoint = allocator.Allocate();
    VerifyOrReturnError(endpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(
        endpoint, provider,
        EndpointComposition(composition.parentId, DataModel::EndpointCompositionPattern::kTree, composition.tagList)));
    mEndpointId = endpoint;

    ReturnErrorOnFailure(RegisterOptionalClusters(endpoint, provider));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    ReturnErrorOnFailure(mEnergyManagement.Register(allocator.Allocate(), provider, EndpointComposition::WithParent(mEndpointId)));
    ReturnErrorOnFailure(
        mGridElectricalSensor.Register(allocator.Allocate(), provider, ChildComposition(mEndpointId, Span(kGridComponentTag))));
    ReturnErrorOnFailure(mBatteryElectricalSensor.Register(allocator.Allocate(), provider,
                                                           ChildComposition(mEndpointId, Span(kBatteryComponentTag))));
    ReturnErrorOnFailure(
        mGridPowerSource.Register(allocator.Allocate(), provider, ChildComposition(mEndpointId, Span(kGridComponentTag))));
    ReturnErrorOnFailure(
        mBatteryPowerSource.Register(allocator.Allocate(), provider, ChildComposition(mEndpointId, Span(kBatteryComponentTag))));

    transaction.Commit();
    return CHIP_NO_ERROR;
}

void BatteryStorage::UnregisterComposedEndpoints(CodeDrivenDataModelProvider & provider)
{
    if (mBatteryPowerSource.GetEndpointId() != kInvalidEndpointId)
    {
        mBatteryPowerSource.Unregister(provider);
    }
    if (mGridPowerSource.GetEndpointId() != kInvalidEndpointId)
    {
        mGridPowerSource.Unregister(provider);
    }
    if (mBatteryElectricalSensor.GetEndpointId() != kInvalidEndpointId)
    {
        mBatteryElectricalSensor.Unregister(provider);
    }
    if (mGridElectricalSensor.GetEndpointId() != kInvalidEndpointId)
    {
        mGridElectricalSensor.Unregister(provider);
    }
    if (mEnergyManagement.GetEndpointId() != kInvalidEndpointId)
    {
        mEnergyManagement.Unregister(provider);
    }
}

void BatteryStorage::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterComposedEndpoints(provider);
    if (mEndpointId != kInvalidEndpointId)
    {
        UnregisterDescriptor(mEndpointId, provider);
        mEndpointId = kInvalidEndpointId;
    }
    UnregisterOptionalClusters(provider);
}

} // namespace chip::app
