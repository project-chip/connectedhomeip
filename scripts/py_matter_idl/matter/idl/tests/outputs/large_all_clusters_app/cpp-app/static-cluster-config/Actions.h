// DO NOT EDIT - Generated file
//
// Application configuration for Actions based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/Actions/AttributeIds.h>
#include <clusters/Actions/CommandIds.h>
#include <clusters/Actions/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Actions {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::ActionList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::EndpointLists::Id,
    Attributes::FeatureMap::Id,
    Attributes::SetupURL::Id,
};

inline constexpr CommandId kEndpoint1EnabledCommands[] = {
    Commands::DisableAction::Id,
    Commands::DisableActionWithDuration::Id,
    Commands::EnableAction::Id,
    Commands::EnableActionWithDuration::Id,
    Commands::InstantAction::Id,
    Commands::InstantActionWithTransition::Id,
    Commands::PauseAction::Id,
    Commands::PauseActionWithDuration::Id,
    Commands::ResumeAction::Id,
    Commands::StartAction::Id,
    Commands::StartActionWithDuration::Id,
    Commands::StopAction::Id,
};

} // namespace detail

using FeatureBitmapType = Clusters::StaticApplicationConfig::NoFeatureFlagsDefined;

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
    case Attributes::ActionList::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::EndpointLists::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::SetupURL::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::DisableAction::Id:
    case Commands::DisableActionWithDuration::Id:
    case Commands::EnableAction::Id:
    case Commands::EnableActionWithDuration::Id:
    case Commands::InstantAction::Id:
    case Commands::InstantActionWithTransition::Id:
    case Commands::PauseAction::Id:
    case Commands::PauseActionWithDuration::Id:
    case Commands::ResumeAction::Id:
    case Commands::StartAction::Id:
    case Commands::StartActionWithDuration::Id:
    case Commands::StopAction::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip

