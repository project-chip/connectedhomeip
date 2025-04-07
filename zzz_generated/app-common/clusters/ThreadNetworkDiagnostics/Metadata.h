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
namespace clusters {
namespace ThreadNetworkDiagnostics {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {
namespace Channel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::Channel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Channel
namespace RoutingRole {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RoutingRole::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RoutingRole
namespace NetworkName {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::NetworkName::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NetworkName
namespace PanId {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::PanId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PanId
namespace ExtendedPanId {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::ExtendedPanId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ExtendedPanId
namespace MeshLocalPrefix {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::MeshLocalPrefix::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MeshLocalPrefix
namespace OverrunCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::OverrunCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OverrunCount
namespace NeighborTable {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::NeighborTable::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NeighborTable
namespace RouteTable {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RouteTable::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RouteTable
namespace PartitionId {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::PartitionId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PartitionId
namespace Weighting {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::Weighting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Weighting
namespace DataVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::DataVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DataVersion
namespace StableDataVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::StableDataVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace StableDataVersion
namespace LeaderRouterId {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::LeaderRouterId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LeaderRouterId
namespace DetachedRoleCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::DetachedRoleCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DetachedRoleCount
namespace ChildRoleCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::ChildRoleCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ChildRoleCount
namespace RouterRoleCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RouterRoleCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RouterRoleCount
namespace LeaderRoleCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::LeaderRoleCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LeaderRoleCount
namespace AttachAttemptCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::AttachAttemptCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AttachAttemptCount
namespace PartitionIdChangeCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::PartitionIdChangeCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PartitionIdChangeCount
namespace BetterPartitionAttachAttemptCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::BetterPartitionAttachAttemptCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BetterPartitionAttachAttemptCount
namespace ParentChangeCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::ParentChangeCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ParentChangeCount
namespace TxTotalCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxTotalCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxTotalCount
namespace TxUnicastCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxUnicastCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxUnicastCount
namespace TxBroadcastCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxBroadcastCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxBroadcastCount
namespace TxAckRequestedCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxAckRequestedCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxAckRequestedCount
namespace TxAckedCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxAckedCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxAckedCount
namespace TxNoAckRequestedCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxNoAckRequestedCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxNoAckRequestedCount
namespace TxDataCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxDataCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxDataCount
namespace TxDataPollCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxDataPollCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxDataPollCount
namespace TxBeaconCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxBeaconCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxBeaconCount
namespace TxBeaconRequestCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxBeaconRequestCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxBeaconRequestCount
namespace TxOtherCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxOtherCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxOtherCount
namespace TxRetryCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxRetryCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxRetryCount
namespace TxDirectMaxRetryExpiryCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxDirectMaxRetryExpiryCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxDirectMaxRetryExpiryCount
namespace TxIndirectMaxRetryExpiryCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxIndirectMaxRetryExpiryCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxIndirectMaxRetryExpiryCount
namespace TxErrCcaCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxErrCcaCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxErrCcaCount
namespace TxErrAbortCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxErrAbortCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxErrAbortCount
namespace TxErrBusyChannelCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::TxErrBusyChannelCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxErrBusyChannelCount
namespace RxTotalCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxTotalCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxTotalCount
namespace RxUnicastCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxUnicastCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxUnicastCount
namespace RxBroadcastCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxBroadcastCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxBroadcastCount
namespace RxDataCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxDataCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxDataCount
namespace RxDataPollCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxDataPollCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxDataPollCount
namespace RxBeaconCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxBeaconCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxBeaconCount
namespace RxBeaconRequestCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxBeaconRequestCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxBeaconRequestCount
namespace RxOtherCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxOtherCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxOtherCount
namespace RxAddressFilteredCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxAddressFilteredCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxAddressFilteredCount
namespace RxDestAddrFilteredCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxDestAddrFilteredCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxDestAddrFilteredCount
namespace RxDuplicatedCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxDuplicatedCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxDuplicatedCount
namespace RxErrNoFrameCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxErrNoFrameCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxErrNoFrameCount
namespace RxErrUnknownNeighborCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxErrUnknownNeighborCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxErrUnknownNeighborCount
namespace RxErrInvalidSrcAddrCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxErrInvalidSrcAddrCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxErrInvalidSrcAddrCount
namespace RxErrSecCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxErrSecCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxErrSecCount
namespace RxErrFcsCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxErrFcsCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxErrFcsCount
namespace RxErrOtherCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::RxErrOtherCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RxErrOtherCount
namespace ActiveTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::ActiveTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveTimestamp
namespace PendingTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::PendingTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PendingTimestamp
namespace Delay {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::Delay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Delay
namespace SecurityPolicy {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::SecurityPolicy::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SecurityPolicy
namespace ChannelPage0Mask {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::ChannelPage0Mask::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ChannelPage0Mask
namespace OperationalDatasetComponents {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::OperationalDatasetComponents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OperationalDatasetComponents
namespace ActiveNetworkFaultsList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::ActiveNetworkFaultsList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveNetworkFaultsList
namespace ExtAddress {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::ExtAddress::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ExtAddress
namespace Rloc16 {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadNetworkDiagnostics::Attributes::Rloc16::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Rloc16

} // namespace Attributes

namespace Commands {
namespace ResetCounts {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = ThreadNetworkDiagnostics::Commands::ResetCounts::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace ResetCounts

} // namespace Commands
} // namespace ThreadNetworkDiagnostics
} // namespace clusters
} // namespace app
} // namespace chip
