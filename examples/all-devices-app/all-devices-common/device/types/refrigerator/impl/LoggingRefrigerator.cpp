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

#include "LoggingRefrigerator.h"

#include <lib/support/CodeUtils.h>

#include <array>

namespace chip::app {

namespace {

// Distinct positional tags: sibling endpoints of the same device type under one tree root
// must carry mutually-distinct TagList entries (checked by Descriptor 2.2).
const std::array<EndpointComposition::SemanticTag, 7> kCabinetTags = { {
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

LoggingRefrigerator::LoggingRefrigerator(TimerDelegate & timerDelegate) : LoggingRefrigerator(timerDelegate, Config{}) {}

LoggingRefrigerator::LoggingRefrigerator(TimerDelegate & timerDelegate, Config config)
{
    // The DescriptorCluster keeps only a non-owning view of the tag list, so the configured
    // tags must be copied into storage that outlives the registered descriptor.
    mOwnedTags.assign(config.tagList.begin(), config.tagList.end());
    mTagList = Span(mOwnedTags.data(), mOwnedTags.size());

    // The MA-refrigerator device type requires at least one cabinet endpoint.
    VerifyOrDie(config.cabinetCount >= 1);
    // Cabinets beyond one need a distinct positional tag each (see kCabinetTags).
    VerifyOrDie(config.cabinetCount <= kCabinetTags.size());

    // Reserve up front: the parts capture the name pointers at construction, so the
    // name strings must not move afterwards.
    mCabinetNames.reserve(config.cabinetCount);
    mCabinets.reserve(config.cabinetCount);
    for (uint8_t i = 0; i < config.cabinetCount; i++)
    {
        mCabinetNames.push_back("Cabinet " + std::to_string(i + 1));
        mCabinets.push_back(std::make_unique<LoggingTemperatureControlledCabinetPart>(timerDelegate, config.cabinetConfig,
                                                                                      mCabinetNames.back().c_str()));
    }
}

CHIP_ERROR LoggingRefrigerator::RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider)
{
    // A single cabinet needs no tag; multiple cabinets of the same device type under one
    // tree root each need a distinct one.
    const bool tagCabinets = mCabinets.size() > 1;
    for (size_t i = 0; i < mCabinets.size(); i++)
    {
        EndpointComposition composition = EndpointComposition::WithParent(GetEndpointId());
        if (tagCabinets)
        {
            composition.tagList = Span(&kCabinetTags[i], 1);
        }
        ReturnErrorOnFailure(mCabinets[i]->Register(allocator, provider, composition));
    }
    return CHIP_NO_ERROR;
}

void LoggingRefrigerator::UnregisterParts(CodeDrivenDataModelProvider & provider)
{
    // Cabinets whose registration never completed (or was rolled back) still have an
    // invalid endpoint id: skip them, unregistering them again would fail in
    // RemoveEndpoint().
    for (auto it = mCabinets.rbegin(); it != mCabinets.rend(); ++it)
    {
        if ((*it)->GetEndpointId() != kInvalidEndpointId)
        {
            (*it)->Unregister(provider);
        }
    }
}

} // namespace chip::app
