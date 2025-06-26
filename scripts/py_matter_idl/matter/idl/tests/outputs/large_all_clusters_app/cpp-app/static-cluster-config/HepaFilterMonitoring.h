// DO NOT EDIT - Generated file
//
// Application configuration for HepaFilterMonitoring based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/HepaFilterMonitoring/AttributeIds.h>
#include <clusters/HepaFilterMonitoring/CommandIds.h>
#include <clusters/HepaFilterMonitoring/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace HepaFilterMonitoring {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ChangeIndication::Id,
    Attributes::ClusterRevision::Id,
    Attributes::Condition::Id,
    Attributes::DegradationDirection::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::InPlaceIndicator::Id,
    Attributes::LastChangedTime::Id,
    Attributes::ReplacementProductList::Id,
};

inline constexpr CommandId kEndpoint1EnabledCommands[] = {
    Commands::ResetCondition::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
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
    case Attributes::ChangeIndication::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::Condition::Id:
    case Attributes::DegradationDirection::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::InPlaceIndicator::Id:
    case Attributes::LastChangedTime::Id:
    case Attributes::ReplacementProductList::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::ResetCondition::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace HepaFilterMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip

