// DO NOT EDIT - Generated file
//
// Application configuration for DiagnosticLogs based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/DiagnosticLogs/AttributeIds.h>
#include <clusters/DiagnosticLogs/CommandIds.h>
#include <clusters/DiagnosticLogs/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
};

inline constexpr CommandId kEndpoint0EnabledCommands[] = {
    Commands::RetrieveLogsRequest::Id,
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
    case Attributes::FeatureMap::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::RetrieveLogsRequest::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip

