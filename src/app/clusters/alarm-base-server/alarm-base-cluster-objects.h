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

#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/DishwasherAlarm/Attributes.h>
#include <clusters/DishwasherAlarm/Commands.h>
#include <clusters/DishwasherAlarm/Enums.h>
#include <clusters/DishwasherAlarm/Events.h>
#include <clusters/DishwasherAlarm/Metadata.h>
#include <clusters/RefrigeratorAlarm/Metadata.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>

#include <array>
#include <cstdint>

namespace chip::app::Clusters::AlarmBase {

// Generic alarm bitmap type shared by all AlarmBase derivations.
enum class AlarmBitmap : uint32_t
{
    kNone = 0,
};
using AlarmMap = BitMask<AlarmBitmap>;

// A pair of cluster ID and revision.
struct ClusterRevisionEntry
{
    ClusterId id;
    uint32_t revision;
};

static constexpr ClusterRevisionEntry kAliasedClusters[] = {
    { DishwasherAlarm::Id, DishwasherAlarm::kRevision },
    { RefrigeratorAlarm::Id, RefrigeratorAlarm::kRevision },
};

using Feature      = DishwasherAlarm::Feature;
namespace Commands = DishwasherAlarm::Commands;
namespace Events   = DishwasherAlarm::Events;

namespace Attributes {

namespace Mask      = DishwasherAlarm::Attributes::Mask;
namespace Latch     = DishwasherAlarm::Attributes::Latch;
namespace State     = DishwasherAlarm::Attributes::State;
namespace Supported = DishwasherAlarm::Attributes::Supported;

constexpr std::array<DataModel::AttributeEntry, 3> kMandatoryMetadata = {
    Mask::kMetadataEntry,
    State::kMetadataEntry,
    Supported::kMetadataEntry,
};

namespace GeneratedCommandList = DishwasherAlarm::Attributes::GeneratedCommandList;
namespace AcceptedCommandList  = DishwasherAlarm::Attributes::AcceptedCommandList;
namespace AttributeList        = DishwasherAlarm::Attributes::AttributeList;
namespace FeatureMap           = DishwasherAlarm::Attributes::FeatureMap;
namespace ClusterRevision      = DishwasherAlarm::Attributes::ClusterRevision;

} // namespace Attributes

} // namespace chip::app::Clusters::AlarmBase
