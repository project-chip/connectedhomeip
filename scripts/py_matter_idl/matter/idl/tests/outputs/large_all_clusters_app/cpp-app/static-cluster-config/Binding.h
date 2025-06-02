// DO NOT EDIT - Generated file
//
// Application configuration for Binding based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/Binding/AttributeIds.h>
#include <clusters/Binding/CommandIds.h>
#include <clusters/Binding/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Binding {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::Binding::Id,
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
};
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::Binding::Id,
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
};
} // namespace detail

using FeatureBitmapType = Clusters::StaticApplicationConfig::NoFeatureFlagsDefined;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 2> kFixedClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint0EnabledAttributes),
        .enabledCommands = Span<const CommandId>(),
    },
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::Binding::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::FeatureMap::Id:
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
} // namespace Binding
} // namespace Clusters
} // namespace app
} // namespace chip

