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

#include <device/types/solar-power/SolarPower.h>

#include <device/types/solar-power/SolarPowerTags.h>
#include <devices/Types.h>

namespace chip::app {

namespace {

EndpointComposition ChildComposition(EndpointId parent, Span<const EndpointComposition::SemanticTag> tags)
{
    EndpointComposition composition = EndpointComposition::WithParent(parent);
    composition.tagList             = tags;
    return composition;
}

} // namespace

SolarPower::SolarPower(const Context & context, Parameters parameters) :
    DeviceInterface(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kSolarPower, 1)), mContext(context),
    mOutputElectricalSensor(context.timerDelegate, context.testEventTriggerDelegate, {}, parameters.outputSensorRole),
    mGridPowerSource("Solar power grid connection"_span, Clusters::PowerSource::WiredCurrentTypeEnum::kAc, 0)
{}

CHIP_ERROR SolarPower::Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
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

    ReturnErrorOnFailure(mGridPowerSource.Register(allocator.Allocate(), provider,
                                                   ChildComposition(mEndpointId, Span(SolarPowerTags::kGridConnectionTag))));
    ReturnErrorOnFailure(mOutputElectricalSensor.Register(allocator.Allocate(), provider,
                                                          ChildComposition(mEndpointId, Span(SolarPowerTags::kGridConnectionTag))));

    ReturnErrorOnFailure(RegisterAdditionalComposedEndpoints(allocator, provider, mEndpointId));

    transaction.Commit();
    return CHIP_NO_ERROR;
}

void SolarPower::UnregisterMandatoryComposedEndpoints(CodeDrivenDataModelProvider & provider)
{
    if (mOutputElectricalSensor.GetEndpointId() != kInvalidEndpointId)
    {
        mOutputElectricalSensor.Unregister(provider);
    }
    if (mGridPowerSource.GetEndpointId() != kInvalidEndpointId)
    {
        mGridPowerSource.Unregister(provider);
    }
}

void SolarPower::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterAdditionalComposedEndpoints(provider);
    UnregisterMandatoryComposedEndpoints(provider);
    UnregisterOptionalClusters(provider);
    if (mEndpointId != kInvalidEndpointId)
    {
        UnregisterDescriptor(mEndpointId, provider);
        mEndpointId = kInvalidEndpointId;
    }
}

} // namespace chip::app
