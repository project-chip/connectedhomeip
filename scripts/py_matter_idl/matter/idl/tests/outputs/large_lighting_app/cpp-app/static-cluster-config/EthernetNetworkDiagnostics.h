// DO NOT EDIT - Generated file
//
// Application configuration for EthernetNetworkDiagnostics based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/EthernetNetworkDiagnostics/AttributeIds.h>
#include <clusters/EthernetNetworkDiagnostics/CommandIds.h>
#include <clusters/EthernetNetworkDiagnostics/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace EthernetNetworkDiagnostics {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::CarrierDetect::Id,
    Attributes::ClusterRevision::Id,
    Attributes::CollisionCount::Id,
    Attributes::FeatureMap::Id,
    Attributes::FullDuplex::Id,
    Attributes::OverrunCount::Id,
    Attributes::PacketRxCount::Id,
    Attributes::PacketTxCount::Id,
    Attributes::PHYRate::Id,
    Attributes::TimeSinceReset::Id,
    Attributes::TxErrCount::Id,
};

inline constexpr CommandId kEndpoint0EnabledCommands[] = {
    Commands::ResetCounts::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kPacketCounts, // feature bit 0x1
            FeatureBitmapType::kErrorCounts// feature bit 0x2
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint0EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint0EnabledCommands),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::PHYRate::Id:
    case Attributes::CarrierDetect::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::CollisionCount::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::FullDuplex::Id:
    case Attributes::OverrunCount::Id:
    case Attributes::PacketRxCount::Id:
    case Attributes::PacketTxCount::Id:
    case Attributes::TimeSinceReset::Id:
    case Attributes::TxErrCount::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::ResetCounts::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace EthernetNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip

