// DO NOT EDIT - Generated file
//
// Application configuration for SoftwareDiagnostics based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/SoftwareDiagnostics/AttributeIds.h>
#include <clusters/SoftwareDiagnostics/CommandIds.h>
#include <clusters/SoftwareDiagnostics/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace SoftwareDiagnostics {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::ClusterRevision::Id,
    Attributes::CurrentHeapFree::Id,
    Attributes::CurrentHeapHighWatermark::Id,
    Attributes::CurrentHeapUsed::Id,
    Attributes::FeatureMap::Id,
    Attributes::ThreadMetrics::Id,
};

inline constexpr CommandId kEndpoint0EnabledCommands[] = {
    Commands::ResetWatermarks::Id,
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
    case Attributes::ClusterRevision::Id:
    case Attributes::CurrentHeapFree::Id:
    case Attributes::CurrentHeapHighWatermark::Id:
    case Attributes::CurrentHeapUsed::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::ThreadMetrics::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::ResetWatermarks::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace SoftwareDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip

