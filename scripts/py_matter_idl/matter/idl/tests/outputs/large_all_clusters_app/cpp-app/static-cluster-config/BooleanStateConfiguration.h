// DO NOT EDIT - Generated file
//
// Application configuration for BooleanStateConfiguration based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/BooleanStateConfiguration/AttributeIds.h>
#include <clusters/BooleanStateConfiguration/CommandIds.h>
#include <clusters/BooleanStateConfiguration/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace BooleanStateConfiguration {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AlarmsActive::Id,
    Attributes::AlarmsEnabled::Id,
    Attributes::AlarmsSupported::Id,
    Attributes::AlarmsSuppressed::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::CurrentSensitivityLevel::Id,
    Attributes::DefaultSensitivityLevel::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::SensorFault::Id,
    Attributes::SupportedSensitivityLevels::Id,
};

inline constexpr CommandId kEndpoint1EnabledCommands[] = {
    Commands::EnableDisableAlarm::Id,
    Commands::SuppressAlarm::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kVisual, // feature bit 0x1
            FeatureBitmapType::kAudible, // feature bit 0x2
            FeatureBitmapType::kAlarmSuppress, // feature bit 0x4
            FeatureBitmapType::kSensitivityLevel// feature bit 0x8
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint1EnabledCommands),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::AcceptedCommandList::Id:
    case Attributes::AlarmsActive::Id:
    case Attributes::AlarmsEnabled::Id:
    case Attributes::AlarmsSupported::Id:
    case Attributes::AlarmsSuppressed::Id:
    case Attributes::AttributeList::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::CurrentSensitivityLevel::Id:
    case Attributes::DefaultSensitivityLevel::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::SensorFault::Id:
    case Attributes::SupportedSensitivityLevels::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::EnableDisableAlarm::Id:
    case Commands::SuppressAlarm::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace BooleanStateConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip

