// DO NOT EDIT - Generated file
//
// Application configuration for GeneralDiagnostics based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/GeneralDiagnostics/AttributeIds.h>
#include <clusters/GeneralDiagnostics/CommandIds.h>
#include <clusters/GeneralDiagnostics/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralDiagnostics {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::ActiveHardwareFaults::Id,
    Attributes::ActiveNetworkFaults::Id,
    Attributes::ActiveRadioFaults::Id,
    Attributes::AttributeList::Id,
    Attributes::BootReason::Id,
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::NetworkInterfaces::Id,
    Attributes::RebootCount::Id,
    Attributes::TestEventTriggersEnabled::Id,
    Attributes::TotalOperationalHours::Id,
    Attributes::UpTime::Id,
};

inline constexpr CommandId kEndpoint0EnabledCommands[] = {
    Commands::PayloadTestRequest::Id,
    Commands::PayloadTestResponse::Id,
    Commands::TestEventTrigger::Id,
    Commands::TimeSnapshot::Id,
    Commands::TimeSnapshotResponse::Id,
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
    case Attributes::ActiveHardwareFaults::Id:
    case Attributes::ActiveNetworkFaults::Id:
    case Attributes::ActiveRadioFaults::Id:
    case Attributes::AttributeList::Id:
    case Attributes::BootReason::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::NetworkInterfaces::Id:
    case Attributes::RebootCount::Id:
    case Attributes::TestEventTriggersEnabled::Id:
    case Attributes::TotalOperationalHours::Id:
    case Attributes::UpTime::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::PayloadTestRequest::Id:
    case Commands::PayloadTestResponse::Id:
    case Commands::TestEventTrigger::Id:
    case Commands::TimeSnapshot::Id:
    case Commands::TimeSnapshotResponse::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace GeneralDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip

