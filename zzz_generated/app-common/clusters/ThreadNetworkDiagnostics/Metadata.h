// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThreadNetworkDiagnostics (cluster code: 53/0x35)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ThreadNetworkDiagnostics/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadNetworkDiagnostics {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {
namespace Channel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Channel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Channel
namespace RoutingRole {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RoutingRole::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RoutingRole
namespace NetworkName {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NetworkName::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NetworkName
namespace PanId {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PanId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PanId
namespace ExtendedPanId {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ExtendedPanId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ExtendedPanId
namespace MeshLocalPrefix {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MeshLocalPrefix::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MeshLocalPrefix
namespace OverrunCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OverrunCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OverrunCount
namespace NeighborTable {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NeighborTable::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NeighborTable
namespace RouteTable {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RouteTable::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RouteTable
namespace PartitionId {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PartitionId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PartitionId
namespace Weighting {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Weighting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Weighting
namespace DataVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DataVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DataVersion
namespace StableDataVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = StableDataVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace StableDataVersion
namespace LeaderRouterId {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LeaderRouterId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LeaderRouterId
namespace DetachedRoleCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DetachedRoleCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DetachedRoleCount
namespace ChildRoleCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ChildRoleCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ChildRoleCount
namespace RouterRoleCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RouterRoleCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RouterRoleCount
namespace LeaderRoleCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LeaderRoleCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LeaderRoleCount
namespace AttachAttemptCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AttachAttemptCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AttachAttemptCount
namespace PartitionIdChangeCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PartitionIdChangeCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PartitionIdChangeCount
namespace BetterPartitionAttachAttemptCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BetterPartitionAttachAttemptCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BetterPartitionAttachAttemptCount
namespace ParentChangeCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ParentChangeCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ParentChangeCount
namespace TxTotalCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxTotalCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxTotalCount
namespace TxUnicastCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxUnicastCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxUnicastCount
namespace TxBroadcastCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxBroadcastCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxBroadcastCount
namespace TxAckRequestedCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxAckRequestedCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxAckRequestedCount
namespace TxAckedCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxAckedCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxAckedCount
namespace TxNoAckRequestedCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxNoAckRequestedCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxNoAckRequestedCount
namespace TxDataCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxDataCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxDataCount
namespace TxDataPollCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxDataPollCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxDataPollCount
namespace TxBeaconCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxBeaconCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxBeaconCount
namespace TxBeaconRequestCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxBeaconRequestCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxBeaconRequestCount
namespace TxOtherCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxOtherCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxOtherCount
namespace TxRetryCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxRetryCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxRetryCount
namespace TxDirectMaxRetryExpiryCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxDirectMaxRetryExpiryCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxDirectMaxRetryExpiryCount
namespace TxIndirectMaxRetryExpiryCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxIndirectMaxRetryExpiryCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxIndirectMaxRetryExpiryCount
namespace TxErrCcaCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxErrCcaCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxErrCcaCount
namespace TxErrAbortCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxErrAbortCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxErrAbortCount
namespace TxErrBusyChannelCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxErrBusyChannelCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxErrBusyChannelCount
namespace RxTotalCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxTotalCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxTotalCount
namespace RxUnicastCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxUnicastCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxUnicastCount
namespace RxBroadcastCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxBroadcastCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxBroadcastCount
namespace RxDataCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxDataCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxDataCount
namespace RxDataPollCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxDataPollCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxDataPollCount
namespace RxBeaconCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxBeaconCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxBeaconCount
namespace RxBeaconRequestCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxBeaconRequestCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxBeaconRequestCount
namespace RxOtherCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxOtherCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxOtherCount
namespace RxAddressFilteredCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxAddressFilteredCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxAddressFilteredCount
namespace RxDestAddrFilteredCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxDestAddrFilteredCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxDestAddrFilteredCount
namespace RxDuplicatedCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxDuplicatedCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxDuplicatedCount
namespace RxErrNoFrameCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxErrNoFrameCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxErrNoFrameCount
namespace RxErrUnknownNeighborCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxErrUnknownNeighborCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxErrUnknownNeighborCount
namespace RxErrInvalidSrcAddrCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxErrInvalidSrcAddrCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxErrInvalidSrcAddrCount
namespace RxErrSecCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxErrSecCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxErrSecCount
namespace RxErrFcsCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxErrFcsCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxErrFcsCount
namespace RxErrOtherCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RxErrOtherCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxErrOtherCount
namespace ActiveTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveTimestamp
namespace PendingTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PendingTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PendingTimestamp
namespace Delay {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Delay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Delay
namespace SecurityPolicy {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SecurityPolicy::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SecurityPolicy
namespace ChannelPage0Mask {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ChannelPage0Mask::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ChannelPage0Mask
namespace OperationalDatasetComponents {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OperationalDatasetComponents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OperationalDatasetComponents
namespace ActiveNetworkFaultsList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveNetworkFaultsList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveNetworkFaultsList
namespace ExtAddress {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ExtAddress::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ExtAddress
namespace Rloc16 {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Rloc16::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Rloc16

} // namespace Attributes

namespace Commands {
namespace ResetCounts {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ResetCounts::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace ResetCounts

} // namespace Commands
} // namespace ThreadNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
