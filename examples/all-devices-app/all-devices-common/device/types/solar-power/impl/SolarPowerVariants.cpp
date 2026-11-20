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

#include <device/types/solar-power/impl/SolarPowerVariants.h>

#include <clusters/shared/Enums.h>

namespace chip::app {
namespace {

using Role        = SimulatedElectricalSensor::Role;
using PanelConfig = SimulatedSolarPower::PanelConfig;
using NS          = Clusters::Globals::SemanticTagNamespaceId;

constexpr uint8_t kTagAcPhase1 = 0x02;
constexpr uint8_t kTagAcPhase2 = 0x03;
constexpr uint8_t kTagAcPhase3 = 0x04;

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

const EndpointComposition::SemanticTag kBaselineTag[] = {
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = to_underlying(Clusters::Globals::SemanticTagNamespaceId::kNumber),
      .tag         = 1 },
};

SimulatedSolarPower::Config BaselineConfig()
{
    return {
        .tags = Span(kBaselineTag),
    };
}

const EndpointComposition::SemanticTag kFourParallelPanelsTag[] = {
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = to_underlying(Clusters::Globals::SemanticTagNamespaceId::kNumber),
      .tag         = 2 },
};

SimulatedSolarPower::Config FourParallelPanelsConfig()
{
    static const PanelConfig kParallelPanels[] = {
        { .tags = ParallelPanel1Tag(), .role = Role::kBattery },
        { .tags = ParallelPanel2Tag(), .role = Role::kBattery },
        { .tags = ParallelPanel3Tag(), .role = Role::kBattery },
        { .tags = ParallelPanel4Tag(), .role = Role::kBattery },
    };
    return {
        .panels = Span(kParallelPanels),
        .tags   = Span(kFourParallelPanelsTag),
    };
}

const EndpointComposition::SemanticTag kTwoStringsTag[] = {
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = to_underlying(Clusters::Globals::SemanticTagNamespaceId::kNumber),
      .tag         = 3 },
};

SimulatedSolarPower::Config TwoStringsConfig()
{
    static const PanelConfig kStrings[] = {
        { .tags = ParallelPanel1Tag(), .role = Role::kBattery },
        { .tags = ParallelPanel2Tag(), .role = Role::kBattery },
    };
    return {
        .panels = Span(kStrings),
        .tags   = Span(kTwoStringsTag),
    };
}

const EndpointComposition::SemanticTag kTwoStringsFourPanelsTag[] = {
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = to_underlying(Clusters::Globals::SemanticTagNamespaceId::kNumber),
      .tag         = 4 },
};

SimulatedSolarPower::Config TwoStringsFourPanelsConfig()
{
    static const PanelConfig kStringPanels[] = {
        { .tags = String1Panel1Tags(), .role = Role::kBattery },
        { .tags = String1Panel2Tags(), .role = Role::kBattery },
        { .tags = String2Panel1Tags(), .role = Role::kBattery },
        { .tags = String2Panel2Tags(), .role = Role::kBattery },
    };
    return {
        .panels = Span(kStringPanels),
        .tags   = Span(kTwoStringsFourPanelsTag),
    };
}

const EndpointComposition::SemanticTag kThreePhaseTag[] = {
    { .mfgCode     = DataModel::NullNullable,
      .namespaceID = to_underlying(Clusters::Globals::SemanticTagNamespaceId::kNumber),
      .tag         = 5 },
};

SimulatedSolarPower::Config ThreePhaseConfig()
{
    static const PanelConfig kPhaseSensors[] = {
        { .tags = PhaseATag(), .role = Role::kGrid, .withUserLabel = false },
        { .tags = PhaseBTag(), .role = Role::kGrid, .withUserLabel = false },
        { .tags = PhaseCTag(), .role = Role::kGrid, .withUserLabel = false },
    };
    return {
        .outputSensorRole = Role::kAcPolyphaseOutput,
        .panels           = Span(kPhaseSensors),
        .tags             = Span(kThreePhaseTag),
    };
}

// Keep the solar-power creator count in zephyr/CMakeLists.txt in sync with this table.
constexpr SolarPowerVariant kVariants[] = {
    { "solar-power", &BaselineConfig },
    { "solar-power-four-panels", &FourParallelPanelsConfig },
    { "solar-power-two-strings", &TwoStringsConfig },
    { "solar-power-two-strings-four-panels", &TwoStringsFourPanelsConfig },
    { "solar-power-three-phase", &ThreePhaseConfig },
};

} // namespace

Span<const SolarPowerVariant> SolarPowerVariants()
{
    return Span(kVariants);
}

} // namespace chip::app
