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

#include "LoggingCooktop.h"

namespace chip::app {

// LoggingCookSurfacePart

LoggingCookSurfacePart::LoggingCookSurfacePart(TimerDelegate & timerDelegate, PlatformIdentifyIntegration & platformIdentify,
                                               const char * name) :
    CookSurfacePart(timerDelegate, *this, *this, platformIdentify), mPlatformIdentify(platformIdentify), mName(name)
{}

void LoggingCookSurfacePart::OnOnOffChanged(bool on)
{
    ChipLogProgress(DeviceLayer, "CookSurface (%s): OnOnOffChanged -> %s", mName, on ? "ON" : "OFF");
}

void LoggingCookSurfacePart::OnOffStartup(bool on)
{
    ChipLogProgress(DeviceLayer, "CookSurface (%s): OnOffStartup -> %s", mName, on ? "ON" : "OFF");
}

void LoggingCookSurfacePart::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "CookSurface (%s): OnIdentifyStart", mName);
    mPlatformIdentify.NotifyIdentifyStart(cluster);
}

void LoggingCookSurfacePart::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "CookSurface (%s): OnIdentifyStop", mName);
    mPlatformIdentify.NotifyIdentifyStop(cluster);
}

void LoggingCookSurfacePart::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "CookSurface (%s): OnTriggerEffect", mName);
    mPlatformIdentify.NotifyTriggerEffect(cluster);
}

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

// LoggingCooktop

LoggingCooktop::LoggingCooktop(TimerDelegate & timerDelegate, PlatformIdentifyIntegration & platformIdentify) :
    mSurface1(timerDelegate, platformIdentify, "Left"), mSurface2(timerDelegate, platformIdentify, "Right")
{}

CHIP_ERROR LoggingCooktop::RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider)
{
    ReturnErrorOnFailure(mSurface1.Register(
        allocator.Allocate(), provider,
        EndpointComposition(GetEndpointId(), DataModel::EndpointCompositionPattern::kFullFamily, Span(&kSurface1Tag, 1))));
    ReturnErrorOnFailure(mSurface2.Register(
        allocator.Allocate(), provider,
        EndpointComposition(GetEndpointId(), DataModel::EndpointCompositionPattern::kFullFamily, Span(&kSurface2Tag, 1))));
    return CHIP_NO_ERROR;
}

void LoggingCooktop::UnregisterParts(CodeDrivenDataModelProvider & provider)
{
    mSurface2.Unregister(provider);
    mSurface1.Unregister(provider);
}

} // namespace chip::app
