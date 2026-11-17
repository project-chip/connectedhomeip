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

namespace chip::app::SolarPowerTags {

namespace {

using NS = Clusters::Globals::SemanticTagNamespaceId;

// Namespace Electrical Measurement (0x000A); see Namespace-ElectricalMeasurement.xml.
constexpr uint8_t kTagAcPhase1 = 0x02;
constexpr uint8_t kTagAcPhase2 = 0x03;
constexpr uint8_t kTagAcPhase3 = 0x04;

} // namespace

Span<const EndpointComposition::SemanticTag> GridConnectionTag()
{
    static const EndpointComposition::SemanticTag kTag[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kPowerSource), .tag = 0x01 },
    };
    return Span(kTag);
}

Span<const EndpointComposition::SemanticTag> InverterTemperatureTag()
{
    static const EndpointComposition::SemanticTag kTag[] = {
        { .mfgCode     = DataModel::NullNullable,
          .namespaceID = to_underlying(NS::kLocation),
          .tag         = static_cast<uint8_t>(Clusters::Globals::LocationTag::kOutdoor) },
    };
    return Span(kTag);
}

Span<const EndpointComposition::SemanticTag> ParallelPanel1Tag()
{
    static const EndpointComposition::SemanticTag kTag[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 1 },
    };
    return Span(kTag);
}

Span<const EndpointComposition::SemanticTag> ParallelPanel2Tag()
{
    static const EndpointComposition::SemanticTag kTag[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 2 },
    };
    return Span(kTag);
}

Span<const EndpointComposition::SemanticTag> ParallelPanel3Tag()
{
    static const EndpointComposition::SemanticTag kTag[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 3 },
    };
    return Span(kTag);
}

Span<const EndpointComposition::SemanticTag> ParallelPanel4Tag()
{
    static const EndpointComposition::SemanticTag kTag[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 4 },
    };
    return Span(kTag);
}

Span<const EndpointComposition::SemanticTag> String1Tag()
{
    static const EndpointComposition::SemanticTag kTag[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 1 },
    };
    return Span(kTag);
}

Span<const EndpointComposition::SemanticTag> String2Tag()
{
    static const EndpointComposition::SemanticTag kTag[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 2 },
    };
    return Span(kTag);
}

// Figure 19: two strings of two panels — string index (Number) plus panel index within the string (Position).
Span<const EndpointComposition::SemanticTag> String1Panel1Tags()
{
    static const EndpointComposition::SemanticTag kTags[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 1 },
        { .mfgCode     = DataModel::NullNullable,
          .namespaceID = to_underlying(NS::kPosition),
          .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kLeft) },
    };
    return Span(kTags);
}

Span<const EndpointComposition::SemanticTag> String1Panel2Tags()
{
    static const EndpointComposition::SemanticTag kTags[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 1 },
        { .mfgCode     = DataModel::NullNullable,
          .namespaceID = to_underlying(NS::kPosition),
          .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kRight) },
    };
    return Span(kTags);
}

Span<const EndpointComposition::SemanticTag> String2Panel1Tags()
{
    static const EndpointComposition::SemanticTag kTags[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 2 },
        { .mfgCode     = DataModel::NullNullable,
          .namespaceID = to_underlying(NS::kPosition),
          .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kLeft) },
    };
    return Span(kTags);
}

Span<const EndpointComposition::SemanticTag> String2Panel2Tags()
{
    static const EndpointComposition::SemanticTag kTags[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kNumber), .tag = 2 },
        { .mfgCode     = DataModel::NullNullable,
          .namespaceID = to_underlying(NS::kPosition),
          .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kRight) },
    };
    return Span(kTags);
}

Span<const EndpointComposition::SemanticTag> PhaseATag()
{
    static const EndpointComposition::SemanticTag kTag[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kElectricalMeasurement), .tag = kTagAcPhase1 },
    };
    return Span(kTag);
}

Span<const EndpointComposition::SemanticTag> PhaseBTag()
{
    static const EndpointComposition::SemanticTag kTag[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kElectricalMeasurement), .tag = kTagAcPhase2 },
    };
    return Span(kTag);
}

Span<const EndpointComposition::SemanticTag> PhaseCTag()
{
    static const EndpointComposition::SemanticTag kTag[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kElectricalMeasurement), .tag = kTagAcPhase3 },
    };
    return Span(kTag);
}

} // namespace chip::app::SolarPowerTags
