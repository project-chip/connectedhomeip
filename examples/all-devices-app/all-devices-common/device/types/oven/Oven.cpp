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

#include "Oven.h"

#include <devices/Types.h>

namespace chip::app {

Oven::Oven(TimerDelegate & timerDelegate, Clusters::OnOffDelegate & surfaceOnOff, Clusters::IdentifyDelegate & cavityIdentify,
           Clusters::IdentifyDelegate & surfaceIdentify, const Config & config) :
    DeviceInterface(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kOven, 1)),
    mCavity(timerDelegate, config.cavityConfig, cavityIdentify), mSurface(timerDelegate, surfaceOnOff, surfaceIdentify)
{}

CHIP_ERROR Oven::Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    mEndpointId = allocator.Allocate();

    ReturnErrorOnFailure(RegisterDescriptor(
        mEndpointId, provider,
        EndpointComposition(composition.parentId, DataModel::EndpointCompositionPattern::kTree, composition.tagList)));
    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    ReturnErrorOnFailure(mCavity.Register(allocator, provider, EndpointComposition::WithParent(mEndpointId)));
    ReturnErrorOnFailure(mSurface.Register(allocator, provider, EndpointComposition::WithParent(mEndpointId)));

    transaction.Commit();
    return CHIP_NO_ERROR;
}

void Oven::Unregister(CodeDrivenDataModelProvider & provider)
{
    mSurface.Unregister(provider);
    mCavity.Unregister(provider);
    UnregisterDescriptor(mEndpointId, provider);
    mEndpointId = kInvalidEndpointId;
}

} // namespace chip::app
