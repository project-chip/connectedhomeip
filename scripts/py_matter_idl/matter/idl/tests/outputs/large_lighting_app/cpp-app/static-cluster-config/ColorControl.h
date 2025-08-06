// DO NOT EDIT - Generated file
//
// Application configuration for ColorControl based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/ColorControl/AttributeIds.h>
#include <clusters/ColorControl/CommandIds.h>
#include <clusters/ColorControl/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace ColorControl {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::ColorCapabilities::Id,
    Attributes::ColorLoopActive::Id,
    Attributes::ColorLoopDirection::Id,
    Attributes::ColorLoopStartEnhancedHue::Id,
    Attributes::ColorLoopStoredEnhancedHue::Id,
    Attributes::ColorLoopTime::Id,
    Attributes::ColorMode::Id,
    Attributes::ColorTemperatureMireds::Id,
    Attributes::ColorTempPhysicalMaxMireds::Id,
    Attributes::ColorTempPhysicalMinMireds::Id,
    Attributes::CoupleColorTempToLevelMinMireds::Id,
    Attributes::CurrentHue::Id,
    Attributes::CurrentSaturation::Id,
    Attributes::CurrentX::Id,
    Attributes::CurrentY::Id,
    Attributes::EnhancedColorMode::Id,
    Attributes::EnhancedCurrentHue::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::NumberOfPrimaries::Id,
    Attributes::Options::Id,
    Attributes::RemainingTime::Id,
    Attributes::StartUpColorTemperatureMireds::Id,
};

inline constexpr CommandId kEndpoint1EnabledCommands[] = {
    Commands::ColorLoopSet::Id,
    Commands::EnhancedMoveHue::Id,
    Commands::EnhancedMoveToHue::Id,
    Commands::EnhancedMoveToHueAndSaturation::Id,
    Commands::EnhancedStepHue::Id,
    Commands::MoveColor::Id,
    Commands::MoveColorTemperature::Id,
    Commands::MoveHue::Id,
    Commands::MoveSaturation::Id,
    Commands::MoveToColor::Id,
    Commands::MoveToColorTemperature::Id,
    Commands::MoveToHue::Id,
    Commands::MoveToHueAndSaturation::Id,
    Commands::MoveToSaturation::Id,
    Commands::StepColor::Id,
    Commands::StepColorTemperature::Id,
    Commands::StepHue::Id,
    Commands::StepSaturation::Id,
    Commands::StopMoveStep::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kHueAndSaturation, // feature bit 0x1
            FeatureBitmapType::kEnhancedHue, // feature bit 0x2
            FeatureBitmapType::kColorLoop, // feature bit 0x4
            FeatureBitmapType::kXY, // feature bit 0x8
            FeatureBitmapType::kColorTemperature// feature bit 0x10
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint1EnabledCommands),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::AcceptedCommandList::Id:
    case Attributes::AttributeList::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::ColorCapabilities::Id:
    case Attributes::ColorLoopActive::Id:
    case Attributes::ColorLoopDirection::Id:
    case Attributes::ColorLoopStartEnhancedHue::Id:
    case Attributes::ColorLoopStoredEnhancedHue::Id:
    case Attributes::ColorLoopTime::Id:
    case Attributes::ColorMode::Id:
    case Attributes::ColorTempPhysicalMaxMireds::Id:
    case Attributes::ColorTempPhysicalMinMireds::Id:
    case Attributes::ColorTemperatureMireds::Id:
    case Attributes::CoupleColorTempToLevelMinMireds::Id:
    case Attributes::CurrentHue::Id:
    case Attributes::CurrentSaturation::Id:
    case Attributes::CurrentX::Id:
    case Attributes::CurrentY::Id:
    case Attributes::EnhancedColorMode::Id:
    case Attributes::EnhancedCurrentHue::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::NumberOfPrimaries::Id:
    case Attributes::Options::Id:
    case Attributes::RemainingTime::Id:
    case Attributes::StartUpColorTemperatureMireds::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::ColorLoopSet::Id:
    case Commands::EnhancedMoveHue::Id:
    case Commands::EnhancedMoveToHue::Id:
    case Commands::EnhancedMoveToHueAndSaturation::Id:
    case Commands::EnhancedStepHue::Id:
    case Commands::MoveColor::Id:
    case Commands::MoveColorTemperature::Id:
    case Commands::MoveHue::Id:
    case Commands::MoveSaturation::Id:
    case Commands::MoveToColor::Id:
    case Commands::MoveToColorTemperature::Id:
    case Commands::MoveToHue::Id:
    case Commands::MoveToHueAndSaturation::Id:
    case Commands::MoveToSaturation::Id:
    case Commands::StepColor::Id:
    case Commands::StepColorTemperature::Id:
    case Commands::StepHue::Id:
    case Commands::StepSaturation::Id:
    case Commands::StopMoveStep::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace ColorControl
} // namespace Clusters
} // namespace app
} // namespace chip

