// DO NOT EDIT - Generated file
//
// Application configuration for AccessControl based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/AccessControl/AttributeIds.h>
#include <clusters/AccessControl/CommandIds.h>
#include <clusters/AccessControl/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace AccessControl {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AccessControlEntriesPerFabric::Id,
    Attributes::Acl::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::Extension::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::SubjectsPerAccessControlEntry::Id,
    Attributes::TargetsPerAccessControlEntry::Id,
};
} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint0EnabledAttributes),
        .enabledCommands = Span<const CommandId>(),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::AcceptedCommandList::Id:
    case Attributes::AccessControlEntriesPerFabric::Id:
    case Attributes::Acl::Id:
    case Attributes::AttributeList::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::Extension::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::SubjectsPerAccessControlEntry::Id:
    case Attributes::TargetsPerAccessControlEntry::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace AccessControl
} // namespace Clusters
} // namespace app
} // namespace chip

