// DO NOT EDIT - Generated file
//
// Application configuration for DishwasherAlarm based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/DishwasherAlarm/AttributeIds.h>
#include <clusters/DishwasherAlarm/CommandIds.h>
#include <clusters/DishwasherAlarm/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace DishwasherAlarm {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::Latch::Id,
    Attributes::Mask::Id,
    Attributes::State::Id,
    Attributes::Supported::Id,
};

inline constexpr CommandId kEndpoint1EnabledCommands[] = {
    Commands::ModifyEnabledAlarms::Id,
    Commands::Reset::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kReset// feature bit 0x1
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
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::Latch::Id:
    case Attributes::Mask::Id:
    case Attributes::State::Id:
    case Attributes::Supported::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::ModifyEnabledAlarms::Id:
    case Commands::Reset::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace DishwasherAlarm
} // namespace Clusters
} // namespace app
} // namespace chip

