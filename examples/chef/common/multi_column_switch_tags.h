/*
 *
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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/Nullable.h>
#include <app/util/attribute-storage.h>
#include <devices/Types.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include "DeviceTypes.h"

namespace chef {

namespace {

using SemanticTagStructType = chip::app::Clusters::Descriptor::Structs::SemanticTagStruct::Type;

// Namespaces
constexpr uint8_t kNamespaceCommonPosition = 8;
constexpr uint8_t kNamespaceSwitches       = 67;

// Tags
constexpr uint8_t kTagCommonPositionColumn = 6;
constexpr uint8_t kTagSwitchesToggle       = 2;
constexpr uint8_t kTagSwitchesUp           = 3;
constexpr uint8_t kTagSwitchesDown         = 4;
constexpr uint8_t kTagSwitchesNext         = 5;
constexpr uint8_t kTagSwitchesPrevious     = 6;
constexpr uint8_t kTagSwitchesCustom       = 8;

inline const SemanticTagStructType kUpTags[] = {
    { .namespaceID = kNamespaceCommonPosition,
      .tag         = kTagCommonPositionColumn,
      .label       = chip::MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>(chip::CharSpan("Column", 6))) },
    { .namespaceID = kNamespaceSwitches,
      .tag         = kTagSwitchesUp,
      .label       = chip::MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>(chip::CharSpan("Up", 2))) },
    { .namespaceID = kNamespaceSwitches,
      .tag         = kTagSwitchesCustom,
      .label       = chip::MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>(chip::CharSpan("Custom", 6))) }
};

inline const SemanticTagStructType kDownTags[] = {
    { .namespaceID = kNamespaceCommonPosition,
      .tag         = kTagCommonPositionColumn,
      .label       = chip::MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>(chip::CharSpan("Column", 6))) },
    { .namespaceID = kNamespaceSwitches,
      .tag         = kTagSwitchesDown,
      .label       = chip::MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>(chip::CharSpan("Down", 4))) },
    { .namespaceID = kNamespaceSwitches,
      .tag         = kTagSwitchesCustom,
      .label       = chip::MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>(chip::CharSpan("Custom", 6))) }
};

inline const SemanticTagStructType kToggleTags[] = {
    { .namespaceID = kNamespaceCommonPosition,
      .tag         = kTagCommonPositionColumn,
      .label       = chip::MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>(chip::CharSpan("Column", 6))) },
    { .namespaceID = kNamespaceSwitches,
      .tag         = kTagSwitchesToggle,
      .label       = chip::MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>(chip::CharSpan("Toggle", 6))) },
    { .namespaceID = kNamespaceSwitches,
      .tag         = kTagSwitchesPrevious,
      .label       = chip::MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>(chip::CharSpan("Previous", 8))) },
    { .namespaceID = kNamespaceSwitches,
      .tag         = kTagSwitchesNext,
      .label       = chip::MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>(chip::CharSpan("Next", 4))) },
    { .namespaceID = kNamespaceSwitches,
      .tag         = kTagSwitchesCustom,
      .label       = chip::MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>(chip::CharSpan("Custom", 6))) }
};

} // namespace

/**
 * Endpoint to tag list mapping for Multi-Column Switch. Indexable by (endpointId - 1) for endpoints 1-9.
 */
inline const chip::Span<const SemanticTagStructType> kMultiColumnSwitchTags[] = {
    chip::Span<const SemanticTagStructType>(kUpTags),     // Endpoint 1
    chip::Span<const SemanticTagStructType>(kDownTags),   // Endpoint 2
    chip::Span<const SemanticTagStructType>(kToggleTags), // Endpoint 3
    chip::Span<const SemanticTagStructType>(kUpTags),     // Endpoint 4
    chip::Span<const SemanticTagStructType>(kDownTags),   // Endpoint 5
    chip::Span<const SemanticTagStructType>(kToggleTags), // Endpoint 6
    chip::Span<const SemanticTagStructType>(kUpTags),     // Endpoint 7
    chip::Span<const SemanticTagStructType>(kDownTags),   // Endpoint 8
    chip::Span<const SemanticTagStructType>(kToggleTags), // Endpoint 9
};

/**
 * Returns true if endpoints 1-9 all have generic switch device type.
 */
inline bool isMultiColumnSwitch()
{
    chip::EndpointId num_endpoints = MATTER_ARRAY_SIZE(kMultiColumnSwitchTags);
    // Check if this is the multi column switch.
    for (chip::EndpointId ep = 1; ep <= num_endpoints; ++ep)
    {
        if (!DeviceTypes::EndpointHasDeviceType(ep, chip::app::Device::kGenericSwitchDeviceTypeId))
            return false;
    }
    return true;
}

/**
 * Initialises the multi column switch app. Sets semantic tags.
 */
inline void InitMultiColumnSwitch()
{
    if (!isMultiColumnSwitch())
        return;

    chip::EndpointId num_endpoints = MATTER_ARRAY_SIZE(kMultiColumnSwitchTags);

    for (chip::EndpointId ep = 1; ep <= num_endpoints; ++ep)
    {
        LogErrorOnFailure(SetTagList(ep, kMultiColumnSwitchTags[ep - 1]));
    }
}

} // namespace chef
