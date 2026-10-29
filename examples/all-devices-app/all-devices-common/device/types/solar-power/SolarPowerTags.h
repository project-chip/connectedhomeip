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

#pragma once

#include <clusters/shared/Enums.h>
#include <device/api/Interface.h>
#include <lib/support/Span.h>

namespace chip::app::SolarPowerTags {

inline constexpr EndpointComposition::SemanticTag kGridConnectionTag[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kPowerSourceId, .tag = 0x01 },
};

inline constexpr EndpointComposition::SemanticTag kInverterTemperatureTag[] = {
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = CommonNamespace::kLocationId,
      .tag         = static_cast<uint8_t>(Clusters::Globals::LocationTag::kOutdoor) },
};

inline constexpr EndpointComposition::SemanticTag kParallelPanel1Tag[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kNumberId, .tag = 1 },
};
inline constexpr EndpointComposition::SemanticTag kParallelPanel2Tag[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kNumberId, .tag = 2 },
};
inline constexpr EndpointComposition::SemanticTag kParallelPanel3Tag[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kNumberId, .tag = 3 },
};
inline constexpr EndpointComposition::SemanticTag kParallelPanel4Tag[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kNumberId, .tag = 4 },
};

inline constexpr EndpointComposition::SemanticTag kString1Tag[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kNumberId, .tag = 1 },
};
inline constexpr EndpointComposition::SemanticTag kString2Tag[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kNumberId, .tag = 2 },
};

// Figure 19: two strings of two panels — string index (Number) plus panel index within the string (Position).
inline constexpr EndpointComposition::SemanticTag kString1Panel1Tags[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kNumberId, .tag = 1 },
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = CommonNamespace::kPositionId,
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kLeft) },
};
inline constexpr EndpointComposition::SemanticTag kString1Panel2Tags[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kNumberId, .tag = 1 },
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = CommonNamespace::kPositionId,
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kRight) },
};
inline constexpr EndpointComposition::SemanticTag kString2Panel1Tags[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kNumberId, .tag = 2 },
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = CommonNamespace::kPositionId,
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kLeft) },
};
inline constexpr EndpointComposition::SemanticTag kString2Panel2Tags[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kNumberId, .tag = 2 },
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = CommonNamespace::kPositionId,
      .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kRight) },
};

inline constexpr EndpointComposition::SemanticTag kPhaseATag[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kElectricalMeasurementId, .tag = 0x01 },
};
inline constexpr EndpointComposition::SemanticTag kPhaseBTag[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kElectricalMeasurementId, .tag = 0x02 },
};
inline constexpr EndpointComposition::SemanticTag kPhaseCTag[] = {
    { .mfgCode = DataModel::NullNullable, .namespaceID = CommonNamespace::kElectricalMeasurementId, .tag = 0x03 },
};

Span<const EndpointComposition::SemanticTag> ParallelPanelTag(uint8_t panelIndex);

Span<const EndpointComposition::SemanticTag> StringPanelTags(uint8_t stringIndex, uint8_t panelIndexInString);

} // namespace chip::app::SolarPowerTags
