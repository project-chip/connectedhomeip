// DO NOT EDIT - Generated file
//
// Application configuration for OtaSoftwareUpdateRequestor based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/OtaSoftwareUpdateRequestor/AttributeIds.h>
#include <clusters/OtaSoftwareUpdateRequestor/CommandIds.h>
#include <clusters/OtaSoftwareUpdateRequestor/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace OtaSoftwareUpdateRequestor {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::ClusterRevision::Id,
    Attributes::DefaultOTAProviders::Id,
    Attributes::FeatureMap::Id,
    Attributes::UpdatePossible::Id,
    Attributes::UpdateState::Id,
    Attributes::UpdateStateProgress::Id,
};

inline constexpr CommandId kEndpoint0EnabledCommands[] = {
    Commands::AnnounceOTAProvider::Id,
};

} // namespace detail

using FeatureBitmapType = Clusters::StaticApplicationConfig::NoFeatureFlagsDefined;

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
    case Attributes::ClusterRevision::Id:
    case Attributes::DefaultOTAProviders::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::UpdatePossible::Id:
    case Attributes::UpdateState::Id:
    case Attributes::UpdateStateProgress::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::AnnounceOTAProvider::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace OtaSoftwareUpdateRequestor
} // namespace Clusters
} // namespace app
} // namespace chip

