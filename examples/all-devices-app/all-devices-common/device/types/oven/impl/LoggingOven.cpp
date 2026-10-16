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

#include "LoggingOven.h"

#include <lib/support/CodeUtils.h>

#include <array>

namespace chip::app {

namespace {

// Distinct positional tags: sibling endpoints of the same device type under one tree root
// must carry mutually-distinct TagList entries (checked by Descriptor 2.2).
const std::array<EndpointComposition::SemanticTag, 7> kCavityTags = { {
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = CommonNamespace::kPositionId,
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kLeft) },
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = CommonNamespace::kPositionId,
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kRight) },
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = CommonNamespace::kPositionId,
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kTop) },
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = CommonNamespace::kPositionId,
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kBottom) },
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = CommonNamespace::kPositionId,
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kMiddle) },
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = CommonNamespace::kPositionId,
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kRow) },
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = CommonNamespace::kPositionId,
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kColumn) },
} };

} // namespace

LoggingOven::LoggingOven(TimerDelegate & timerDelegate) : LoggingOven(timerDelegate, Config{}) {}

LoggingOven::LoggingOven(TimerDelegate & timerDelegate, Config config) : mSurface(timerDelegate, "Top Surface")
{
    // The DescriptorCluster keeps only a non-owning view of the tag list, so the configured
    // tags must be copied into storage that outlives the registered descriptor.
    mOwnedTags.assign(config.tagList.begin(), config.tagList.end());
    mTagList = Span(mOwnedTags.data(), mOwnedTags.size());

    // The MA-oven device type requires at least one cavity endpoint.
    VerifyOrDie(config.cavityCount >= 1);
    // Cavities beyond one need a distinct positional tag each (see kCavityTags).
    VerifyOrDie(config.cavityCount <= kCavityTags.size());

    // Reserve up front: the parts capture the name pointers at construction, so the
    // name strings must not move afterwards.
    mCavityNames.reserve(config.cavityCount);
    mCavities.reserve(config.cavityCount);
    for (uint8_t i = 0; i < config.cavityCount; i++)
    {
        mCavityNames.push_back("Cavity " + std::to_string(i + 1));
        mCavities.push_back(std::make_unique<LoggingTemperatureControlledCabinetPart>(timerDelegate, config.cavityConfig,
                                                                                      mCavityNames.back().c_str()));
    }
}

CHIP_ERROR LoggingOven::RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider)
{
    // A single cavity needs no tag; multiple cavities of the same device type under one
    // tree root each need a distinct one.
    const bool tagCavities = mCavities.size() > 1;
    for (size_t i = 0; i < mCavities.size(); i++)
    {
        EndpointComposition composition = EndpointComposition::WithParent(GetEndpointId());
        if (tagCavities)
        {
            composition.tagList = Span(&kCavityTags[i], 1);
        }
        ReturnErrorOnFailure(mCavities[i]->Register(allocator, provider, composition));
    }
    ReturnErrorOnFailure(mSurface.Register(allocator, provider, EndpointComposition::WithParent(GetEndpointId())));
    return CHIP_NO_ERROR;
}

void LoggingOven::UnregisterParts(CodeDrivenDataModelProvider & provider)
{
    // Parts whose registration never completed (or was rolled back) still have an invalid
    // endpoint id: skip them, unregistering them again would fail in RemoveEndpoint().
    if (mSurface.GetEndpointId() != kInvalidEndpointId)
    {
        mSurface.Unregister(provider);
    }
    for (auto it = mCavities.rbegin(); it != mCavities.rend(); ++it)
    {
        if ((*it)->GetEndpointId() != kInvalidEndpointId)
        {
            (*it)->Unregister(provider);
        }
    }
}

} // namespace chip::app
