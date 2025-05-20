// DO NOT EDIT - Generated file
//
// Application configuration for Descriptor based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <clusters/Descriptor/AttributeIds.h>
#include <clusters/Descriptor/CommandIds.h>
#include <clusters/Descriptor/Enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Descriptor {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClientList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::DeviceTypeList::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::PartsList::Id,
    Attributes::ServerList::Id,
    Attributes::TagList::Id,
};
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClientList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::DeviceTypeList::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::PartsList::Id,
    Attributes::ServerList::Id,
    Attributes::TagList::Id,
};
inline constexpr AttributeId kEndpoint2EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClientList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::DeviceTypeList::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::PartsList::Id,
    Attributes::ServerList::Id,
    Attributes::TagList::Id,
};
inline constexpr AttributeId kEndpoint3EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClientList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::DeviceTypeList::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::PartsList::Id,
    Attributes::ServerList::Id,
    Attributes::TagList::Id,
};
inline constexpr AttributeId kEndpoint4EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClientList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::DeviceTypeList::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::PartsList::Id,
    Attributes::ServerList::Id,
    Attributes::TagList::Id,
};
inline constexpr AttributeId kEndpoint65534EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClientList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::DeviceTypeList::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::PartsList::Id,
    Attributes::ServerList::Id,
};
} // namespace detail

using FeatureBitmapType = Feature;


inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 6> kFixedClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes {detail::kEndpoint0EnabledAttributes},
        .enabledCommands {},
    },
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes {detail::kEndpoint1EnabledAttributes},
        .enabledCommands {},
    },
    {
        .endpointNumber = 2,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes {detail::kEndpoint2EnabledAttributes},
        .enabledCommands {},
    },
    {
        .endpointNumber = 3,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes {detail::kEndpoint3EnabledAttributes},
        .enabledCommands {},
    },
    {
        .endpointNumber = 4,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes {detail::kEndpoint4EnabledAttributes},
        .enabledCommands {},
    },
    {
        .endpointNumber = 65534,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes {detail::kEndpoint65534EnabledAttributes},
        .enabledCommands {},
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::AcceptedCommandList::Id:
    case Attributes::AttributeList::Id:
    case Attributes::ClientList::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::DeviceTypeList::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::PartsList::Id:
    case Attributes::ServerList::Id:
    case Attributes::TagList::Id:
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
} // namespace Descriptor
} // namespace Clusters
} // namespace app
} // namespace chip

