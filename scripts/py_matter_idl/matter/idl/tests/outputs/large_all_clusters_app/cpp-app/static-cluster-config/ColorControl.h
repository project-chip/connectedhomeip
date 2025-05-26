// DO NOT EDIT - Generated file
//
// Application configuration for ColorControl based on EMBER configuration
// from inputs/large_all_clusters_app.matter
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
    Attributes::ClusterRevision::Id,
    Attributes::ColorCapabilities::Id,
    Attributes::ColorLoopActive::Id,
    Attributes::ColorLoopDirection::Id,
    Attributes::ColorLoopStartEnhancedHue::Id,
    Attributes::ColorLoopStoredEnhancedHue::Id,
    Attributes::ColorLoopTime::Id,
    Attributes::ColorMode::Id,
    Attributes::ColorPointBIntensity::Id,
    Attributes::ColorPointBX::Id,
    Attributes::ColorPointBY::Id,
    Attributes::ColorPointGIntensity::Id,
    Attributes::ColorPointGX::Id,
    Attributes::ColorPointGY::Id,
    Attributes::ColorPointRIntensity::Id,
    Attributes::ColorPointRX::Id,
    Attributes::ColorPointRY::Id,
    Attributes::ColorTemperatureMireds::Id,
    Attributes::ColorTempPhysicalMaxMireds::Id,
    Attributes::ColorTempPhysicalMinMireds::Id,
    Attributes::CompensationText::Id,
    Attributes::CoupleColorTempToLevelMinMireds::Id,
    Attributes::CurrentHue::Id,
    Attributes::CurrentSaturation::Id,
    Attributes::CurrentX::Id,
    Attributes::CurrentY::Id,
    Attributes::DriftCompensation::Id,
    Attributes::EnhancedColorMode::Id,
    Attributes::EnhancedCurrentHue::Id,
    Attributes::FeatureMap::Id,
    Attributes::NumberOfPrimaries::Id,
    Attributes::Options::Id,
    Attributes::Primary1Intensity::Id,
    Attributes::Primary1X::Id,
    Attributes::Primary1Y::Id,
    Attributes::Primary2Intensity::Id,
    Attributes::Primary2X::Id,
    Attributes::Primary2Y::Id,
    Attributes::Primary3Intensity::Id,
    Attributes::Primary3X::Id,
    Attributes::Primary3Y::Id,
    Attributes::Primary4Intensity::Id,
    Attributes::Primary4X::Id,
    Attributes::Primary4Y::Id,
    Attributes::Primary5Intensity::Id,
    Attributes::Primary5X::Id,
    Attributes::Primary5Y::Id,
    Attributes::Primary6Intensity::Id,
    Attributes::Primary6X::Id,
    Attributes::Primary6Y::Id,
    Attributes::RemainingTime::Id,
    Attributes::StartUpColorTemperatureMireds::Id,
    Attributes::WhitePointX::Id,
    Attributes::WhitePointY::Id,
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
    case Attributes::ClusterRevision::Id:
    case Attributes::ColorCapabilities::Id:
    case Attributes::ColorLoopActive::Id:
    case Attributes::ColorLoopDirection::Id:
    case Attributes::ColorLoopStartEnhancedHue::Id:
    case Attributes::ColorLoopStoredEnhancedHue::Id:
    case Attributes::ColorLoopTime::Id:
    case Attributes::ColorMode::Id:
    case Attributes::ColorPointBIntensity::Id:
    case Attributes::ColorPointBX::Id:
    case Attributes::ColorPointBY::Id:
    case Attributes::ColorPointGIntensity::Id:
    case Attributes::ColorPointGX::Id:
    case Attributes::ColorPointGY::Id:
    case Attributes::ColorPointRIntensity::Id:
    case Attributes::ColorPointRX::Id:
    case Attributes::ColorPointRY::Id:
    case Attributes::ColorTempPhysicalMaxMireds::Id:
    case Attributes::ColorTempPhysicalMinMireds::Id:
    case Attributes::ColorTemperatureMireds::Id:
    case Attributes::CompensationText::Id:
    case Attributes::CoupleColorTempToLevelMinMireds::Id:
    case Attributes::CurrentHue::Id:
    case Attributes::CurrentSaturation::Id:
    case Attributes::CurrentX::Id:
    case Attributes::CurrentY::Id:
    case Attributes::DriftCompensation::Id:
    case Attributes::EnhancedColorMode::Id:
    case Attributes::EnhancedCurrentHue::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::NumberOfPrimaries::Id:
    case Attributes::Options::Id:
    case Attributes::Primary1Intensity::Id:
    case Attributes::Primary1X::Id:
    case Attributes::Primary1Y::Id:
    case Attributes::Primary2Intensity::Id:
    case Attributes::Primary2X::Id:
    case Attributes::Primary2Y::Id:
    case Attributes::Primary3Intensity::Id:
    case Attributes::Primary3X::Id:
    case Attributes::Primary3Y::Id:
    case Attributes::Primary4Intensity::Id:
    case Attributes::Primary4X::Id:
    case Attributes::Primary4Y::Id:
    case Attributes::Primary5Intensity::Id:
    case Attributes::Primary5X::Id:
    case Attributes::Primary5Y::Id:
    case Attributes::Primary6Intensity::Id:
    case Attributes::Primary6X::Id:
    case Attributes::Primary6Y::Id:
    case Attributes::RemainingTime::Id:
    case Attributes::StartUpColorTemperatureMireds::Id:
    case Attributes::WhitePointX::Id:
    case Attributes::WhitePointY::Id:
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

