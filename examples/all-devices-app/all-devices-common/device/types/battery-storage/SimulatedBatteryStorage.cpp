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

#include <device/types/battery-storage/SimulatedBatteryStorage.h>

#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

namespace {

constexpr char kPowerSourceDescriptionA[] = "Battery storage pack A";
constexpr char kPowerSourceDescriptionB[] = "Battery storage pack B";

// Sibling endpoints of the same device type under one tree root must carry mutually-distinct
// TagList entries (Descriptor 2.2 / TC_DESC_2.2). Spans over these are handed to the device,
// so they must outlive it.
const EndpointComposition::SemanticTag kElectricalSensorTagA[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kNumberId, .tag = 1 },
};
const EndpointComposition::SemanticTag kElectricalSensorTagB[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kNumberId, .tag = 2 },
};
const EndpointComposition::SemanticTag kPowerSourceTagA[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kNumberId, .tag = 1 },
};
const EndpointComposition::SemanticTag kPowerSourceTagB[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kNumberId, .tag = 2 },
};

EndpointComposition ChildComposition(EndpointId parent, Span<const EndpointComposition::SemanticTag> tags)
{
    EndpointComposition composition = EndpointComposition::WithParent(parent);
    composition.tagList             = tags;
    return composition;
}

} // namespace

SimulatedBatteryStorage::SimulatedBatteryStorage(const Context & context) :
    DeviceInterface(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kBatteryStorage, 1)),
    mTimerDelegate(context.timerDelegate), mElectricalSensorA(context.timerDelegate, context.testEventTriggerDelegate),
    mElectricalSensorB(context.timerDelegate, context.testEventTriggerDelegate),
    mPowerSourceA(CharSpan::fromCharString(kPowerSourceDescriptionA), Clusters::PowerSource::BatReplaceabilityEnum::kNotReplaceable,
                  context.timerDelegate, 0),
    mPowerSourceB(CharSpan::fromCharString(kPowerSourceDescriptionB), Clusters::PowerSource::BatReplaceabilityEnum::kNotReplaceable,
                  context.timerDelegate, 1)
{}

CHIP_ERROR SimulatedBatteryStorage::Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
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

    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    ReturnErrorOnFailure(mElectricalSensorA.Register(
        allocator.Allocate(), provider, ChildComposition(mEndpointId, Span(kElectricalSensorTagA))));
    ReturnErrorOnFailure(mElectricalSensorB.Register(
        allocator.Allocate(), provider, ChildComposition(mEndpointId, Span(kElectricalSensorTagB))));
    ReturnErrorOnFailure(mPowerSourceA.Register(allocator.Allocate(), provider,
                                                 ChildComposition(mEndpointId, Span(kPowerSourceTagA))));
    ReturnErrorOnFailure(mPowerSourceB.Register(allocator.Allocate(), provider,
                                                 ChildComposition(mEndpointId, Span(kPowerSourceTagB))));

    transaction.Commit();
    return CHIP_NO_ERROR;
}

void SimulatedBatteryStorage::UnregisterComposedEndpoints(CodeDrivenDataModelProvider & provider)
{
    if (mPowerSourceB.GetEndpointId() != kInvalidEndpointId)
    {
        mPowerSourceB.Unregister(provider);
    }
    if (mPowerSourceA.GetEndpointId() != kInvalidEndpointId)
    {
        mPowerSourceA.Unregister(provider);
    }
    if (mElectricalSensorB.GetEndpointId() != kInvalidEndpointId)
    {
        mElectricalSensorB.Unregister(provider);
    }
    if (mElectricalSensorA.GetEndpointId() != kInvalidEndpointId)
    {
        mElectricalSensorA.Unregister(provider);
    }
}

void SimulatedBatteryStorage::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterComposedEndpoints(provider);
    UnregisterDescriptor(mEndpointId, provider);
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
    mEndpointId = kInvalidEndpointId;
}

} // namespace chip::app
