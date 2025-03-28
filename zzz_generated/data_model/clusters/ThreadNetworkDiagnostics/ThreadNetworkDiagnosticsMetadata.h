// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThreadNetworkDiagnostics (cluster code: 53/0x35)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ThreadNetworkDiagnostics/ThreadNetworkDiagnosticsIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace ThreadNetworkDiagnostics {
namespace Metadata {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kChannelEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::Channel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRoutingRoleEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RoutingRole::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNetworkNameEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::NetworkName::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPanIdEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::PanId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kExtendedPanIdEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::ExtendedPanId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMeshLocalPrefixEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::MeshLocalPrefix::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOverrunCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::OverrunCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNeighborTableEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::NeighborTable::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRouteTableEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RouteTable::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPartitionIdEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::PartitionId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kWeightingEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::Weighting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDataVersionEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::DataVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kStableDataVersionEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::StableDataVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLeaderRouterIdEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::LeaderRouterId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDetachedRoleCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::DetachedRoleCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kChildRoleCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::ChildRoleCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRouterRoleCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RouterRoleCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLeaderRoleCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::LeaderRoleCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAttachAttemptCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::AttachAttemptCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPartitionIdChangeCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::PartitionIdChangeCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kBetterPartitionAttachAttemptCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::BetterPartitionAttachAttemptCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kParentChangeCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::ParentChangeCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxTotalCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxTotalCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxUnicastCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxUnicastCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxBroadcastCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxBroadcastCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxAckRequestedCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxAckRequestedCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxAckedCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxAckedCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxNoAckRequestedCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxNoAckRequestedCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxDataCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxDataCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxDataPollCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxDataPollCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxBeaconCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxBeaconCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxBeaconRequestCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxBeaconRequestCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxOtherCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxOtherCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxRetryCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxRetryCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxDirectMaxRetryExpiryCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxDirectMaxRetryExpiryCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxIndirectMaxRetryExpiryCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxIndirectMaxRetryExpiryCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxErrCcaCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxErrCcaCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxErrAbortCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxErrAbortCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxErrBusyChannelCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxErrBusyChannelCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxTotalCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxTotalCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxUnicastCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxUnicastCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxBroadcastCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxBroadcastCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxDataCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxDataCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxDataPollCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxDataPollCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxBeaconCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxBeaconCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxBeaconRequestCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxBeaconRequestCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxOtherCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxOtherCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxAddressFilteredCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxAddressFilteredCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxDestAddrFilteredCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxDestAddrFilteredCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxDuplicatedCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxDuplicatedCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxErrNoFrameCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxErrNoFrameCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxErrUnknownNeighborCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxErrUnknownNeighborCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxErrInvalidSrcAddrCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxErrInvalidSrcAddrCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxErrSecCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxErrSecCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxErrFcsCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxErrFcsCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRxErrOtherCountEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxErrOtherCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActiveTimestampEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::ActiveTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPendingTimestampEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::PendingTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDelayEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::Delay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSecurityPolicyEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::SecurityPolicy::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kChannelPage0MaskEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::ChannelPage0Mask::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOperationalDatasetComponentsEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::OperationalDatasetComponents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActiveNetworkFaultsListEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::ActiveNetworkFaultsList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kResetCountsEntry = {
    .commandId       = ThreadNetworkDiagnostics::Commands::ResetCounts::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands
} // namespace Metadata
} // namespace ThreadNetworkDiagnostics
} // namespace clusters
} // namespace app
} // namespace chip
