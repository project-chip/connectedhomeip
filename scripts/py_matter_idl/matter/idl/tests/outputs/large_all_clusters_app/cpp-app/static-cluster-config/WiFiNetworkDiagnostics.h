// DO NOT EDIT - Generated file
//
// Application configuration for WiFiNetworkDiagnostics based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/WiFiNetworkDiagnostics/AttributeIds.h>
#include <clusters/WiFiNetworkDiagnostics/CommandIds.h>
#include <clusters/WiFiNetworkDiagnostics/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace WiFiNetworkDiagnostics {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::BeaconLostCount::Id,
    Attributes::BeaconRxCount::Id,
    Attributes::Bssid::Id,
    Attributes::ChannelNumber::Id,
    Attributes::ClusterRevision::Id,
    Attributes::CurrentMaxRate::Id,
    Attributes::FeatureMap::Id,
    Attributes::OverrunCount::Id,
    Attributes::PacketMulticastRxCount::Id,
    Attributes::PacketMulticastTxCount::Id,
    Attributes::PacketUnicastRxCount::Id,
    Attributes::PacketUnicastTxCount::Id,
    Attributes::Rssi::Id,
    Attributes::SecurityType::Id,
    Attributes::WiFiVersion::Id,
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
    case Attributes::BeaconLostCount::Id:
    case Attributes::BeaconRxCount::Id:
    case Attributes::Bssid::Id:
    case Attributes::ChannelNumber::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::CurrentMaxRate::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::OverrunCount::Id:
    case Attributes::PacketMulticastRxCount::Id:
    case Attributes::PacketMulticastTxCount::Id:
    case Attributes::PacketUnicastRxCount::Id:
    case Attributes::PacketUnicastTxCount::Id:
    case Attributes::Rssi::Id:
    case Attributes::SecurityType::Id:
    case Attributes::WiFiVersion::Id:
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
} // namespace WiFiNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip

