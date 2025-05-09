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
inline constexpr DataModel::AttributeEntry kMetadataEntry(Channel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Channel
namespace RoutingRole {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RoutingRole::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RoutingRole
namespace NetworkName {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NetworkName::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NetworkName
namespace PanId {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PanId::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PanId
namespace ExtendedPanId {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ExtendedPanId::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ExtendedPanId
namespace MeshLocalPrefix {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MeshLocalPrefix::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MeshLocalPrefix
namespace OverrunCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OverrunCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OverrunCount
namespace NeighborTable {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(NeighborTable::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace NeighborTable
namespace RouteTable {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(RouteTable::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace RouteTable
namespace PartitionId {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PartitionId::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PartitionId
namespace Weighting {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Weighting::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Weighting
namespace DataVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DataVersion::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace DataVersion
namespace StableDataVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StableDataVersion::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace StableDataVersion
namespace LeaderRouterId {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LeaderRouterId::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace LeaderRouterId
namespace DetachedRoleCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DetachedRoleCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace DetachedRoleCount
namespace ChildRoleCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ChildRoleCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ChildRoleCount
namespace RouterRoleCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RouterRoleCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RouterRoleCount
namespace LeaderRoleCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LeaderRoleCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace LeaderRoleCount
namespace AttachAttemptCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AttachAttemptCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AttachAttemptCount
namespace PartitionIdChangeCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PartitionIdChangeCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PartitionIdChangeCount
namespace BetterPartitionAttachAttemptCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BetterPartitionAttachAttemptCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace BetterPartitionAttachAttemptCount
namespace ParentChangeCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ParentChangeCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ParentChangeCount
namespace TxTotalCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxTotalCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxTotalCount
namespace TxUnicastCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxUnicastCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxUnicastCount
namespace TxBroadcastCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxBroadcastCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxBroadcastCount
namespace TxAckRequestedCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxAckRequestedCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxAckRequestedCount
namespace TxAckedCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxAckedCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxAckedCount
namespace TxNoAckRequestedCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxNoAckRequestedCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxNoAckRequestedCount
namespace TxDataCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxDataCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxDataCount
namespace TxDataPollCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxDataPollCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxDataPollCount
namespace TxBeaconCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxBeaconCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxBeaconCount
namespace TxBeaconRequestCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxBeaconRequestCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxBeaconRequestCount
namespace TxOtherCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxOtherCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxOtherCount
namespace TxRetryCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxRetryCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxRetryCount
namespace TxDirectMaxRetryExpiryCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxDirectMaxRetryExpiryCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace TxDirectMaxRetryExpiryCount
namespace TxIndirectMaxRetryExpiryCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxIndirectMaxRetryExpiryCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace TxIndirectMaxRetryExpiryCount
namespace TxErrCcaCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxErrCcaCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxErrCcaCount
namespace TxErrAbortCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxErrAbortCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxErrAbortCount
namespace TxErrBusyChannelCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxErrBusyChannelCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxErrBusyChannelCount
namespace RxTotalCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxTotalCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxTotalCount
namespace RxUnicastCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxUnicastCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxUnicastCount
namespace RxBroadcastCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxBroadcastCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxBroadcastCount
namespace RxDataCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxDataCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxDataCount
namespace RxDataPollCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxDataPollCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxDataPollCount
namespace RxBeaconCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxBeaconCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxBeaconCount
namespace RxBeaconRequestCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxBeaconRequestCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxBeaconRequestCount
namespace RxOtherCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxOtherCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxOtherCount
namespace RxAddressFilteredCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxAddressFilteredCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxAddressFilteredCount
namespace RxDestAddrFilteredCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxDestAddrFilteredCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxDestAddrFilteredCount
namespace RxDuplicatedCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxDuplicatedCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxDuplicatedCount
namespace RxErrNoFrameCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxErrNoFrameCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxErrNoFrameCount
namespace RxErrUnknownNeighborCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxErrUnknownNeighborCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace RxErrUnknownNeighborCount
namespace RxErrInvalidSrcAddrCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxErrInvalidSrcAddrCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace RxErrInvalidSrcAddrCount
namespace RxErrSecCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxErrSecCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxErrSecCount
namespace RxErrFcsCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxErrFcsCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxErrFcsCount
namespace RxErrOtherCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RxErrOtherCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RxErrOtherCount
namespace ActiveTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ActiveTimestamp::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ActiveTimestamp
namespace PendingTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PendingTimestamp::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PendingTimestamp
namespace Delay {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Delay::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Delay
namespace SecurityPolicy {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SecurityPolicy::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SecurityPolicy
namespace ChannelPage0Mask {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ChannelPage0Mask::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ChannelPage0Mask
namespace OperationalDatasetComponents {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OperationalDatasetComponents::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace OperationalDatasetComponents
namespace ActiveNetworkFaultsList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ActiveNetworkFaultsList::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ActiveNetworkFaultsList
namespace ExtAddress {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ExtAddress::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ExtAddress
namespace Rloc16 {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Rloc16::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Rloc16

} // namespace Attributes

namespace Commands {
namespace ResetCounts {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ResetCounts::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace ResetCounts

} // namespace Commands
} // namespace ThreadNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
