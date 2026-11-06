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

#include <device/types/solar-power/SolarPowerTags.h>

#include <clusters/shared/Enums.h>
#include <lib/support/CodeUtils.h>

namespace chip::app::SolarPowerTags {

namespace {

using NS = Clusters::Globals::SemanticTagNamespaceId;

} // namespace

const EndpointComposition::SemanticTag kGridConnectionTag[1] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kPowerSource), .tag = 0x01 },
};

const EndpointComposition::SemanticTag kInverterTemperatureTag[1] = {
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = to_underlying(NS::kLocation),
      .tag         = static_cast<uint8_t>(Clusters::Globals::LocationTag::kOutdoor) },
};

const EndpointComposition::SemanticTag kParallelPanel1Tag[1] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 1 },
};
const EndpointComposition::SemanticTag kParallelPanel2Tag[1] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 2 },
};
const EndpointComposition::SemanticTag kParallelPanel3Tag[1] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 3 },
};
const EndpointComposition::SemanticTag kParallelPanel4Tag[1] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 4 },
};

const EndpointComposition::SemanticTag kString1Tag[1] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 1 },
};
const EndpointComposition::SemanticTag kString2Tag[1] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 2 },
};

// Figure 19: two strings of two panels — string index (Number) plus panel index within the string (Position).
const EndpointComposition::SemanticTag kString1Panel1Tags[2] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 1 },
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = to_underlying(NS::kPosition),
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kLeft) },
};
const EndpointComposition::SemanticTag kString1Panel2Tags[2] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 1 },
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = to_underlying(NS::kPosition),
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kRight) },
};
const EndpointComposition::SemanticTag kString2Panel1Tags[2] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 2 },
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = to_underlying(NS::kPosition),
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kLeft) },
};
const EndpointComposition::SemanticTag kString2Panel2Tags[2] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 2 },
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = to_underlying(NS::kPosition),
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kRight) },
};

const EndpointComposition::SemanticTag kPhaseATag[1] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kElectricalMeasurement), .tag = 0x01 },
};
const EndpointComposition::SemanticTag kPhaseBTag[1] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kElectricalMeasurement), .tag = 0x02 },
};
const EndpointComposition::SemanticTag kPhaseCTag[1] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kElectricalMeasurement), .tag = 0x03 },
};

Span<const EndpointComposition::SemanticTag> ParallelPanelTag(uint8_t panelIndex)
{
    switch (panelIndex)
    {
    case 1:
        return Span(kParallelPanel1Tag);
    case 2:
        return Span(kParallelPanel2Tag);
    case 3:
        return Span(kParallelPanel3Tag);
    case 4:
        return Span(kParallelPanel4Tag);
    default:
        return Span<const EndpointComposition::SemanticTag>();
    }
}

Span<const EndpointComposition::SemanticTag> StringPanelTags(uint8_t stringIndex, uint8_t panelIndexInString)
{
    if (stringIndex == 1 && panelIndexInString == 1)
    {
        return Span(kString1Panel1Tags);
    }
    if (stringIndex == 1 && panelIndexInString == 2)
    {
        return Span(kString1Panel2Tags);
    }
    if (stringIndex == 2 && panelIndexInString == 1)
    {
        return Span(kString2Panel1Tags);
    }
    if (stringIndex == 2 && panelIndexInString == 2)
    {
        return Span(kString2Panel2Tags);
    }
    return Span<const EndpointComposition::SemanticTag>();
}

} // namespace chip::app::SolarPowerTags
