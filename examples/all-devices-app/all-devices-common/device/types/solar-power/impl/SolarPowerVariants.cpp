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

#include <device/types/solar-power/SolarPowerTags.h>

namespace chip::app {
namespace {

using Role        = SimulatedElectricalSensor::Role;
using PanelConfig = SimulatedSolarPower::PanelConfig;

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
        { .tags = SolarPowerTags::ParallelPanel1Tag(), .role = Role::kBattery },
        { .tags = SolarPowerTags::ParallelPanel2Tag(), .role = Role::kBattery },
        { .tags = SolarPowerTags::ParallelPanel3Tag(), .role = Role::kBattery },
        { .tags = SolarPowerTags::ParallelPanel4Tag(), .role = Role::kBattery },
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
        { .tags = SolarPowerTags::String1Tag(), .role = Role::kBattery },
        { .tags = SolarPowerTags::String2Tag(), .role = Role::kBattery },
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
        { .tags = SolarPowerTags::String1Panel1Tags(), .role = Role::kBattery },
        { .tags = SolarPowerTags::String1Panel2Tags(), .role = Role::kBattery },
        { .tags = SolarPowerTags::String2Panel1Tags(), .role = Role::kBattery },
        { .tags = SolarPowerTags::String2Panel2Tags(), .role = Role::kBattery },
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
        { .tags = SolarPowerTags::PhaseATag(), .role = Role::kGrid },
        { .tags = SolarPowerTags::PhaseBTag(), .role = Role::kGrid },
        { .tags = SolarPowerTags::PhaseCTag(), .role = Role::kGrid },
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
