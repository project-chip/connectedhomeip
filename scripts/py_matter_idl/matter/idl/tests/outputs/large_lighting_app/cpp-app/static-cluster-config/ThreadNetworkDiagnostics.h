// DO NOT EDIT - Generated file
//
// Application configuration for ThreadNetworkDiagnostics based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/ThreadNetworkDiagnostics/AttributeIds.h>
#include <clusters/ThreadNetworkDiagnostics/CommandIds.h>
#include <clusters/ThreadNetworkDiagnostics/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadNetworkDiagnostics {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::ActiveNetworkFaultsList::Id,
    Attributes::ActiveTimestamp::Id,
    Attributes::AttachAttemptCount::Id,
    Attributes::BetterPartitionAttachAttemptCount::Id,
    Attributes::Channel::Id,
    Attributes::ChannelPage0Mask::Id,
    Attributes::ChildRoleCount::Id,
    Attributes::ClusterRevision::Id,
    Attributes::DataVersion::Id,
    Attributes::Delay::Id,
    Attributes::DetachedRoleCount::Id,
    Attributes::ExtendedPanId::Id,
    Attributes::FeatureMap::Id,
    Attributes::LeaderRoleCount::Id,
    Attributes::LeaderRouterId::Id,
    Attributes::MeshLocalPrefix::Id,
    Attributes::NeighborTable::Id,
    Attributes::NetworkName::Id,
    Attributes::OperationalDatasetComponents::Id,
    Attributes::OverrunCount::Id,
    Attributes::PanId::Id,
    Attributes::ParentChangeCount::Id,
    Attributes::PartitionId::Id,
    Attributes::PartitionIdChangeCount::Id,
    Attributes::PendingTimestamp::Id,
    Attributes::RouterRoleCount::Id,
    Attributes::RouteTable::Id,
    Attributes::RoutingRole::Id,
    Attributes::RxAddressFilteredCount::Id,
    Attributes::RxBeaconCount::Id,
    Attributes::RxBeaconRequestCount::Id,
    Attributes::RxBroadcastCount::Id,
    Attributes::RxDataCount::Id,
    Attributes::RxDataPollCount::Id,
    Attributes::RxDestAddrFilteredCount::Id,
    Attributes::RxDuplicatedCount::Id,
    Attributes::RxErrFcsCount::Id,
    Attributes::RxErrInvalidSrcAddrCount::Id,
    Attributes::RxErrNoFrameCount::Id,
    Attributes::RxErrOtherCount::Id,
    Attributes::RxErrSecCount::Id,
    Attributes::RxErrUnknownNeighborCount::Id,
    Attributes::RxOtherCount::Id,
    Attributes::RxTotalCount::Id,
    Attributes::RxUnicastCount::Id,
    Attributes::SecurityPolicy::Id,
    Attributes::StableDataVersion::Id,
    Attributes::TxAckedCount::Id,
    Attributes::TxAckRequestedCount::Id,
    Attributes::TxBeaconCount::Id,
    Attributes::TxBeaconRequestCount::Id,
    Attributes::TxBroadcastCount::Id,
    Attributes::TxDataCount::Id,
    Attributes::TxDataPollCount::Id,
    Attributes::TxDirectMaxRetryExpiryCount::Id,
    Attributes::TxErrAbortCount::Id,
    Attributes::TxErrBusyChannelCount::Id,
    Attributes::TxErrCcaCount::Id,
    Attributes::TxIndirectMaxRetryExpiryCount::Id,
    Attributes::TxNoAckRequestedCount::Id,
    Attributes::TxOtherCount::Id,
    Attributes::TxRetryCount::Id,
    Attributes::TxTotalCount::Id,
    Attributes::TxUnicastCount::Id,
    Attributes::Weighting::Id,
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
            FeatureBitmapType::kErrorCounts, // feature bit 0x2
            FeatureBitmapType::kMLECounts, // feature bit 0x4
            FeatureBitmapType::kMACCounts// feature bit 0x8
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint0EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint0EnabledCommands),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::ActiveNetworkFaultsList::Id:
    case Attributes::ActiveTimestamp::Id:
    case Attributes::AttachAttemptCount::Id:
    case Attributes::BetterPartitionAttachAttemptCount::Id:
    case Attributes::Channel::Id:
    case Attributes::ChannelPage0Mask::Id:
    case Attributes::ChildRoleCount::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::DataVersion::Id:
    case Attributes::Delay::Id:
    case Attributes::DetachedRoleCount::Id:
    case Attributes::ExtendedPanId::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::LeaderRoleCount::Id:
    case Attributes::LeaderRouterId::Id:
    case Attributes::MeshLocalPrefix::Id:
    case Attributes::NeighborTable::Id:
    case Attributes::NetworkName::Id:
    case Attributes::OperationalDatasetComponents::Id:
    case Attributes::OverrunCount::Id:
    case Attributes::PanId::Id:
    case Attributes::ParentChangeCount::Id:
    case Attributes::PartitionId::Id:
    case Attributes::PartitionIdChangeCount::Id:
    case Attributes::PendingTimestamp::Id:
    case Attributes::RouteTable::Id:
    case Attributes::RouterRoleCount::Id:
    case Attributes::RoutingRole::Id:
    case Attributes::RxAddressFilteredCount::Id:
    case Attributes::RxBeaconCount::Id:
    case Attributes::RxBeaconRequestCount::Id:
    case Attributes::RxBroadcastCount::Id:
    case Attributes::RxDataCount::Id:
    case Attributes::RxDataPollCount::Id:
    case Attributes::RxDestAddrFilteredCount::Id:
    case Attributes::RxDuplicatedCount::Id:
    case Attributes::RxErrFcsCount::Id:
    case Attributes::RxErrInvalidSrcAddrCount::Id:
    case Attributes::RxErrNoFrameCount::Id:
    case Attributes::RxErrOtherCount::Id:
    case Attributes::RxErrSecCount::Id:
    case Attributes::RxErrUnknownNeighborCount::Id:
    case Attributes::RxOtherCount::Id:
    case Attributes::RxTotalCount::Id:
    case Attributes::RxUnicastCount::Id:
    case Attributes::SecurityPolicy::Id:
    case Attributes::StableDataVersion::Id:
    case Attributes::TxAckRequestedCount::Id:
    case Attributes::TxAckedCount::Id:
    case Attributes::TxBeaconCount::Id:
    case Attributes::TxBeaconRequestCount::Id:
    case Attributes::TxBroadcastCount::Id:
    case Attributes::TxDataCount::Id:
    case Attributes::TxDataPollCount::Id:
    case Attributes::TxDirectMaxRetryExpiryCount::Id:
    case Attributes::TxErrAbortCount::Id:
    case Attributes::TxErrBusyChannelCount::Id:
    case Attributes::TxErrCcaCount::Id:
    case Attributes::TxIndirectMaxRetryExpiryCount::Id:
    case Attributes::TxNoAckRequestedCount::Id:
    case Attributes::TxOtherCount::Id:
    case Attributes::TxRetryCount::Id:
    case Attributes::TxTotalCount::Id:
    case Attributes::TxUnicastCount::Id:
    case Attributes::Weighting::Id:
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
} // namespace ThreadNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip

