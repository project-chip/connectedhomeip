// DO NOT EDIT - Generated file
//
// Application configuration for NetworkCommissioning based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/NetworkCommissioning/AttributeIds.h>
#include <clusters/NetworkCommissioning/CommandIds.h>
#include <clusters/NetworkCommissioning/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::ConnectMaxTimeSeconds::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::InterfaceEnabled::Id,
    Attributes::LastConnectErrorValue::Id,
    Attributes::LastNetworkID::Id,
    Attributes::LastNetworkingStatus::Id,
    Attributes::MaxNetworks::Id,
    Attributes::Networks::Id,
    Attributes::ScanMaxTimeSeconds::Id,
    Attributes::SupportedThreadFeatures::Id,
    Attributes::SupportedWiFiBands::Id,
    Attributes::ThreadVersion::Id,
};

inline constexpr CommandId kEndpoint0EnabledCommands[] = {
    Commands::AddOrUpdateThreadNetwork::Id,
    Commands::AddOrUpdateWiFiNetwork::Id,
    Commands::ConnectNetwork::Id,
    Commands::ConnectNetworkResponse::Id,
    Commands::NetworkConfigResponse::Id,
    Commands::RemoveNetwork::Id,
    Commands::ReorderNetwork::Id,
    Commands::ScanNetworks::Id,
    Commands::ScanNetworksResponse::Id,
};

inline constexpr AttributeId kEndpoint65534EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::ConnectMaxTimeSeconds::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::InterfaceEnabled::Id,
    Attributes::LastConnectErrorValue::Id,
    Attributes::LastNetworkID::Id,
    Attributes::LastNetworkingStatus::Id,
    Attributes::MaxNetworks::Id,
    Attributes::Networks::Id,
    Attributes::ScanMaxTimeSeconds::Id,
};

inline constexpr CommandId kEndpoint65534EnabledCommands[] = {
    Commands::AddOrUpdateThreadNetwork::Id,
    Commands::AddOrUpdateWiFiNetwork::Id,
    Commands::ConnectNetwork::Id,
    Commands::ConnectNetworkResponse::Id,
    Commands::NetworkConfigResponse::Id,
    Commands::RemoveNetwork::Id,
    Commands::ReorderNetwork::Id,
    Commands::ScanNetworks::Id,
    Commands::ScanNetworksResponse::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 2> kFixedClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kThreadNetworkInterface// feature bit 0x2
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint0EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint0EnabledCommands),
    },
    {
        .endpointNumber = 65534,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint65534EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint65534EnabledCommands),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::AcceptedCommandList::Id:
    case Attributes::AttributeList::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::ConnectMaxTimeSeconds::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::InterfaceEnabled::Id:
    case Attributes::LastConnectErrorValue::Id:
    case Attributes::LastNetworkID::Id:
    case Attributes::LastNetworkingStatus::Id:
    case Attributes::MaxNetworks::Id:
    case Attributes::Networks::Id:
    case Attributes::ScanMaxTimeSeconds::Id:
    case Attributes::SupportedThreadFeatures::Id:
    case Attributes::SupportedWiFiBands::Id:
    case Attributes::ThreadVersion::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::AddOrUpdateThreadNetwork::Id:
    case Commands::AddOrUpdateWiFiNetwork::Id:
    case Commands::ConnectNetwork::Id:
    case Commands::ConnectNetworkResponse::Id:
    case Commands::NetworkConfigResponse::Id:
    case Commands::RemoveNetwork::Id:
    case Commands::ReorderNetwork::Id:
    case Commands::ScanNetworks::Id:
    case Commands::ScanNetworksResponse::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip

