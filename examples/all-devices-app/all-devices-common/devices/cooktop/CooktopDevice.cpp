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

#include "CooktopDevice.h"

#include <devices/Types.h>

namespace chip::app {
namespace {

const Clusters::Globals::Structs::SemanticTagStruct::Type kSurface1Tag = {
    .mfgCode     = DataModel::NullNullable,
    .namespaceID = 7, // Common Position
    .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kLeft),
};

const Clusters::Globals::Structs::SemanticTagStruct::Type kSurface2Tag = {
    .mfgCode     = DataModel::NullNullable,
    .namespaceID = 7, // Common Position
    .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kRight),
};

} // namespace

CooktopDevice::CooktopDevice(TimerDelegate & timerDelegate) :
    DeviceInterface(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kCooktop, 1)), mSurface1(timerDelegate),
    mSurface2(timerDelegate)
{}

CHIP_ERROR CooktopDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    mEndpointId = endpoint;

    ReturnErrorOnFailure(
        InitEndpointRegistration(endpoint, provider, EndpointComposition(parentId, DataModel::EndpointCompositionPattern::kTree)));
    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    ReturnErrorOnFailure(mSurface1.Register(
        endpoint + 1, provider,
        EndpointComposition(endpoint, DataModel::EndpointCompositionPattern::kFullFamily, Span(&kSurface1Tag, 1))));
    ReturnErrorOnFailure(mSurface2.Register(
        endpoint + 2, provider,
        EndpointComposition(endpoint, DataModel::EndpointCompositionPattern::kFullFamily, Span(&kSurface2Tag, 1))));

    return CHIP_NO_ERROR;
}

void CooktopDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    mSurface2.Unregister(provider);
    mSurface1.Unregister(provider);

    ShutdownEndpointRegistration(mEndpointId, provider);
}

} // namespace chip::app
