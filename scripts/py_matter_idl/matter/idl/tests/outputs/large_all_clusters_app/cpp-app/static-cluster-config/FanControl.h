// DO NOT EDIT - Generated file
//
// Application configuration for FanControl based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/FanControl/AttributeIds.h>
#include <clusters/FanControl/CommandIds.h>
#include <clusters/FanControl/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace FanControl {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AirflowDirection::Id,
    Attributes::ClusterRevision::Id,
    Attributes::FanMode::Id,
    Attributes::FanModeSequence::Id,
    Attributes::FeatureMap::Id,
    Attributes::PercentCurrent::Id,
    Attributes::PercentSetting::Id,
    Attributes::RockSetting::Id,
    Attributes::RockSupport::Id,
    Attributes::SpeedCurrent::Id,
    Attributes::SpeedMax::Id,
    Attributes::SpeedSetting::Id,
    Attributes::WindSetting::Id,
    Attributes::WindSupport::Id,
};

inline constexpr CommandId kEndpoint1EnabledCommands[] = {
    Commands::Step::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kMultiSpeed, // feature bit 0x1
            FeatureBitmapType::kAuto, // feature bit 0x2
            FeatureBitmapType::kRocking, // feature bit 0x4
            FeatureBitmapType::kWind, // feature bit 0x8
            FeatureBitmapType::kStep, // feature bit 0x10
            FeatureBitmapType::kAirflowDirection// feature bit 0x20
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint1EnabledCommands),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::AirflowDirection::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::FanMode::Id:
    case Attributes::FanModeSequence::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::PercentCurrent::Id:
    case Attributes::PercentSetting::Id:
    case Attributes::RockSetting::Id:
    case Attributes::RockSupport::Id:
    case Attributes::SpeedCurrent::Id:
    case Attributes::SpeedMax::Id:
    case Attributes::SpeedSetting::Id:
    case Attributes::WindSetting::Id:
    case Attributes::WindSupport::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::Step::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace chip

