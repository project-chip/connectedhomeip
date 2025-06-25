// DO NOT EDIT - Generated file
//
// Application configuration for Groups based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/Groups/AttributeIds.h>
#include <clusters/Groups/CommandIds.h>
#include <clusters/Groups/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Groups {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
    Attributes::NameSupport::Id,
};

inline constexpr CommandId kEndpoint0EnabledCommands[] = {
    Commands::AddGroup::Id,
    Commands::AddGroupIfIdentifying::Id,
    Commands::AddGroupResponse::Id,
    Commands::GetGroupMembership::Id,
    Commands::GetGroupMembershipResponse::Id,
    Commands::RemoveAllGroups::Id,
    Commands::RemoveGroup::Id,
    Commands::RemoveGroupResponse::Id,
    Commands::ViewGroup::Id,
    Commands::ViewGroupResponse::Id,
};

inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::NameSupport::Id,
};

inline constexpr CommandId kEndpoint1EnabledCommands[] = {
    Commands::AddGroup::Id,
    Commands::AddGroupIfIdentifying::Id,
    Commands::AddGroupResponse::Id,
    Commands::GetGroupMembership::Id,
    Commands::GetGroupMembershipResponse::Id,
    Commands::RemoveAllGroups::Id,
    Commands::RemoveGroup::Id,
    Commands::RemoveGroupResponse::Id,
    Commands::ViewGroup::Id,
    Commands::ViewGroupResponse::Id,
};

inline constexpr AttributeId kEndpoint2EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::NameSupport::Id,
};

inline constexpr CommandId kEndpoint2EnabledCommands[] = {
    Commands::AddGroup::Id,
    Commands::AddGroupIfIdentifying::Id,
    Commands::AddGroupResponse::Id,
    Commands::GetGroupMembership::Id,
    Commands::GetGroupMembershipResponse::Id,
    Commands::RemoveAllGroups::Id,
    Commands::RemoveGroup::Id,
    Commands::RemoveGroupResponse::Id,
    Commands::ViewGroup::Id,
    Commands::ViewGroupResponse::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 3> kFixedClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint0EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint0EnabledCommands),
    },
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint1EnabledCommands),
    },
    {
        .endpointNumber = 2,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint2EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint2EnabledCommands),
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
    case Attributes::NameSupport::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::AddGroup::Id:
    case Commands::AddGroupIfIdentifying::Id:
    case Commands::AddGroupResponse::Id:
    case Commands::GetGroupMembership::Id:
    case Commands::GetGroupMembershipResponse::Id:
    case Commands::RemoveAllGroups::Id:
    case Commands::RemoveGroup::Id:
    case Commands::RemoveGroupResponse::Id:
    case Commands::ViewGroup::Id:
    case Commands::ViewGroupResponse::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace Groups
} // namespace Clusters
} // namespace app
} // namespace chip

