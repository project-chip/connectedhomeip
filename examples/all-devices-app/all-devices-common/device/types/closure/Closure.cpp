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

#include "Closure.h"
#include <clusters/shared/Enums.h>
#include <devices/Types.h>

namespace {

CHIP_ERROR ValidateClosureTagList(chip::Span<const chip::app::EndpointComposition::SemanticTag> tags)
{
    size_t closureTagCount = 0;
    for (auto & tag : tags)
    {
        VerifyOrReturnError(tag.namespaceID != chip::app::CommonNamespace::kClosurePanelId, CHIP_ERROR_INVALID_ARGUMENT);
        if (tag.namespaceID == chip::app::CommonNamespace::kClosureId)
        {
            ++closureTagCount;
        }
    }
    VerifyOrReturnError(closureTagCount == 1, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

using SemanticTag = chip::app::Clusters::Globals::Structs::SemanticTagStruct::Type;
using chip::app::Clusters::Globals::ClosureTag;

bool IsAccess(chip::Span<const SemanticTag> tags)
{
    for (const auto & tag : tags)
    {
        if (tag.namespaceID == chip::app::CommonNamespace::kClosureId &&
            (chip::to_underlying(ClosureTag::kWindow) == tag.tag || chip::to_underlying(ClosureTag::kDoor) == tag.tag ||
             chip::to_underlying(ClosureTag::kBarrier) == tag.tag || chip::to_underlying(ClosureTag::kGarageDoor) == tag.tag ||
             chip::to_underlying(ClosureTag::kGate) == tag.tag))
        {
            return true;
        }
    }
    return false;
}

} // namespace

namespace chip {
namespace app {

Closure::Closure(Config config, TimerDelegate & Tdelegate, Clusters::IdentifyDelegate & Idelegate,
                 Clusters::ClosureControl::ClosureControlClusterDelegate & CCdelegate) :
    DeviceInterface(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kClosure, 1)), mConfig(config), mTimerDelegate(Tdelegate),
    mIdentifyDelegate(Idelegate), mClosureControlClusterDelegate(CCdelegate)
{}

CHIP_ERROR Closure::Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                             EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    Span<const EndpointComposition::SemanticTag> tags = composition.tagList.empty() ? mConfig.tags : composition.tagList;
    composition.tagList                               = tags;
    ReturnErrorOnFailure(ValidateClosureTagList(tags));
    DeviceRegistrationTransaction transaction(*this, provider);
    mEndpointId = allocator.Allocate();
    ReturnErrorOnFailure(RegisterDescriptor(mEndpointId, provider, composition));

    Clusters::ClosureControl::ClosureControlCluster::Config CCconfig(mEndpointId, mClosureControlClusterDelegate, mTimerDelegate);

    CCconfig.WithInitialOverallCurrentState(mConfig.initialOverallCurrentState);

    if (mConfig.withAccess || IsAccess(tags) || RegistersAccessDevicePanel())
    {
        CCconfig.WithAccess();
    }
    if (mConfig.withCalibration)
    {
        CCconfig.WithCalibration();
    }
    if (mConfig.withInstantaneous)
    {
        CCconfig.WithInstantaneous();
    }
    if (mConfig.withManuallyOperable)
    {
        CCconfig.WithManuallyOperable();
    }
    if (mConfig.motionLatching.has_value())
    {
        CCconfig.WithMotionLatching(mConfig.motionLatching.value());
    }
    if (mConfig.withPedestrian)
    {
        CCconfig.WithPedestrian();
    }
    if (mConfig.withSpeed)
    {
        CCconfig.WithSpeed();
    }
    if (mConfig.withPositioning)
    {
        CCconfig.WithPositioning();
    }
    if (mConfig.withProtection)
    {
        CCconfig.WithProtection();
    }
    if (mConfig.withVentilation)
    {
        CCconfig.WithVentilation();
    }

    mClosureControlCluster.Create(CCconfig);

    ReturnErrorOnFailure(provider.AddCluster(mClosureControlCluster.Registration()));

    Clusters::IdentifyCluster::Config Iconfig(mEndpointId, mTimerDelegate);
    Iconfig.WithDelegate(&mIdentifyDelegate);
    mIdentifyCluster.Create(Iconfig);

    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    ReturnErrorOnFailure(RegisterParts(allocator, provider, composition));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void Closure::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterParts(provider);
    UnregisterDescriptor(mEndpointId, provider);
    mEndpointId = kInvalidEndpointId;

    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
    if (mClosureControlCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mClosureControlCluster.Cluster()));
        mClosureControlCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
