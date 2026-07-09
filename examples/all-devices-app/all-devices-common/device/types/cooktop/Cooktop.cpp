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

#include "Cooktop.h"

#include <devices/Types.h>

namespace chip::app {
namespace {

const Clusters::Globals::Structs::SemanticTagStruct::Type kSurface1Tag = {
    .mfgCode     = DataModel::NullNullable,
    .namespaceID = CommonNamespace::kPositionId,
    .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kLeft),
};

const Clusters::Globals::Structs::SemanticTagStruct::Type kSurface2Tag = {
    .mfgCode     = DataModel::NullNullable,
    .namespaceID = CommonNamespace::kPositionId,
    .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kRight),
};

} // namespace

// CookSurfacePart

CookSurfacePart::CookSurfacePart(TimerDelegate & timerDelegate, Clusters::OnOffDelegate & onOffDelegate,
                                 Clusters::IdentifyDelegate & identifyDelegate) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kCookSurface, 1)),
    mTimerDelegate(timerDelegate), mOnOffDelegate(onOffDelegate), mIdentifyDelegate(identifyDelegate)
{}

CHIP_ERROR CookSurfacePart::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mTimerDelegate).WithDelegate(&mIdentifyDelegate));
    mOnOffCluster.Create(endpoint, Clusters::OnOffCluster::Context{ .timerDelegate = mTimerDelegate });
    mOnOffCluster.Cluster().AddDelegate(&mOnOffDelegate);

    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));
    ReturnErrorOnFailure(provider.AddCluster(mOnOffCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void CookSurfacePart::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
    if (mOnOffCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mOnOffCluster.Cluster()));
        mOnOffCluster.Cluster().RemoveDelegate(&mOnOffDelegate);
        mOnOffCluster.Destroy();
    }
}

// Cooktop

Cooktop::Cooktop(TimerDelegate & timerDelegate, Clusters::OnOffDelegate & surface1OnOff, Clusters::OnOffDelegate & surface2OnOff,
                 Clusters::IdentifyDelegate & surface1Identify, Clusters::IdentifyDelegate & surface2Identify) :
    DeviceInterface(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kCooktop, 1)),
    mSurface1(timerDelegate, surface1OnOff, surface1Identify), mSurface2(timerDelegate, surface2OnOff, surface2Identify)
{}

CHIP_ERROR Cooktop::Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                             EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    mEndpointId = allocator.Allocate();

    ReturnErrorOnFailure(RegisterDescriptor(
        mEndpointId, provider,
        EndpointComposition(composition.parentId, DataModel::EndpointCompositionPattern::kTree, composition.tagList)));
    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    ReturnErrorOnFailure(mSurface1.Register(
        allocator.Allocate(), provider,
        EndpointComposition(mEndpointId, DataModel::EndpointCompositionPattern::kFullFamily, Span(&kSurface1Tag, 1))));
    ReturnErrorOnFailure(mSurface2.Register(
        allocator.Allocate(), provider,
        EndpointComposition(mEndpointId, DataModel::EndpointCompositionPattern::kFullFamily, Span(&kSurface2Tag, 1))));

    transaction.Commit();
    return CHIP_NO_ERROR;
}

void Cooktop::Unregister(CodeDrivenDataModelProvider & provider)
{
    mSurface2.Unregister(provider);
    mSurface1.Unregister(provider);

    UnregisterDescriptor(mEndpointId, provider);
    mEndpointId = kInvalidEndpointId;
}

} // namespace chip::app
