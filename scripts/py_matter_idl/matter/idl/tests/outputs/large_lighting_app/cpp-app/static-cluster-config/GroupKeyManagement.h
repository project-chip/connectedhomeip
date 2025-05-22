// DO NOT EDIT - Generated file
//
// Application configuration for GroupKeyManagement based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/GroupKeyManagement/AttributeIds.h>
#include <clusters/GroupKeyManagement/CommandIds.h>
#include <clusters/GroupKeyManagement/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace GroupKeyManagement {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::GroupKeyMap::Id,
    Attributes::GroupTable::Id,
    Attributes::MaxGroupKeysPerFabric::Id,
    Attributes::MaxGroupsPerFabric::Id,
};

inline constexpr CommandId kEndpoint0EnabledCommands[] = {
    Commands::KeySetRead::Id,
    Commands::KeySetReadAllIndices::Id,
    Commands::KeySetReadAllIndicesResponse::Id,
    Commands::KeySetReadResponse::Id,
    Commands::KeySetRemove::Id,
    Commands::KeySetWrite::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint0EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint0EnabledCommands),
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
    case Attributes::GroupKeyMap::Id:
    case Attributes::GroupTable::Id:
    case Attributes::MaxGroupKeysPerFabric::Id:
    case Attributes::MaxGroupsPerFabric::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::KeySetRead::Id:
    case Commands::KeySetReadAllIndices::Id:
    case Commands::KeySetReadAllIndicesResponse::Id:
    case Commands::KeySetReadResponse::Id:
    case Commands::KeySetRemove::Id:
    case Commands::KeySetWrite::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace GroupKeyManagement
} // namespace Clusters
} // namespace app
} // namespace chip

