// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ThreadNetworkDiagnostics (cluster code: 53/0x35)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadNetworkDiagnostics {
namespace Attributes {
namespace Channel {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Channel

namespace RoutingRole {
inline constexpr AttributeId Id = 0x00000001;
} // namespace RoutingRole

namespace NetworkName {
inline constexpr AttributeId Id = 0x00000002;
} // namespace NetworkName

namespace PanId {
inline constexpr AttributeId Id = 0x00000003;
} // namespace PanId

namespace ExtendedPanId {
inline constexpr AttributeId Id = 0x00000004;
} // namespace ExtendedPanId

namespace MeshLocalPrefix {
inline constexpr AttributeId Id = 0x00000005;
} // namespace MeshLocalPrefix

namespace OverrunCount {
inline constexpr AttributeId Id = 0x00000006;
} // namespace OverrunCount

namespace NeighborTable {
inline constexpr AttributeId Id = 0x00000007;
} // namespace NeighborTable

namespace RouteTable {
inline constexpr AttributeId Id = 0x00000008;
} // namespace RouteTable

namespace PartitionId {
inline constexpr AttributeId Id = 0x00000009;
} // namespace PartitionId

namespace Weighting {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace Weighting

namespace DataVersion {
inline constexpr AttributeId Id = 0x0000000B;
} // namespace DataVersion

namespace StableDataVersion {
inline constexpr AttributeId Id = 0x0000000C;
} // namespace StableDataVersion

namespace LeaderRouterId {
inline constexpr AttributeId Id = 0x0000000D;
} // namespace LeaderRouterId

namespace DetachedRoleCount {
inline constexpr AttributeId Id = 0x0000000E;
} // namespace DetachedRoleCount

namespace ChildRoleCount {
inline constexpr AttributeId Id = 0x0000000F;
} // namespace ChildRoleCount

namespace RouterRoleCount {
inline constexpr AttributeId Id = 0x00000010;
} // namespace RouterRoleCount

namespace LeaderRoleCount {
inline constexpr AttributeId Id = 0x00000011;
} // namespace LeaderRoleCount

namespace AttachAttemptCount {
inline constexpr AttributeId Id = 0x00000012;
} // namespace AttachAttemptCount

namespace PartitionIdChangeCount {
inline constexpr AttributeId Id = 0x00000013;
} // namespace PartitionIdChangeCount

namespace BetterPartitionAttachAttemptCount {
inline constexpr AttributeId Id = 0x00000014;
} // namespace BetterPartitionAttachAttemptCount

namespace ParentChangeCount {
inline constexpr AttributeId Id = 0x00000015;
} // namespace ParentChangeCount

namespace TxTotalCount {
inline constexpr AttributeId Id = 0x00000016;
} // namespace TxTotalCount

namespace TxUnicastCount {
inline constexpr AttributeId Id = 0x00000017;
} // namespace TxUnicastCount

namespace TxBroadcastCount {
inline constexpr AttributeId Id = 0x00000018;
} // namespace TxBroadcastCount

namespace TxAckRequestedCount {
inline constexpr AttributeId Id = 0x00000019;
} // namespace TxAckRequestedCount

namespace TxAckedCount {
inline constexpr AttributeId Id = 0x0000001A;
} // namespace TxAckedCount

namespace TxNoAckRequestedCount {
inline constexpr AttributeId Id = 0x0000001B;
} // namespace TxNoAckRequestedCount

namespace TxDataCount {
inline constexpr AttributeId Id = 0x0000001C;
} // namespace TxDataCount

namespace TxDataPollCount {
inline constexpr AttributeId Id = 0x0000001D;
} // namespace TxDataPollCount

namespace TxBeaconCount {
inline constexpr AttributeId Id = 0x0000001E;
} // namespace TxBeaconCount

namespace TxBeaconRequestCount {
inline constexpr AttributeId Id = 0x0000001F;
} // namespace TxBeaconRequestCount

namespace TxOtherCount {
inline constexpr AttributeId Id = 0x00000020;
} // namespace TxOtherCount

namespace TxRetryCount {
inline constexpr AttributeId Id = 0x00000021;
} // namespace TxRetryCount

namespace TxDirectMaxRetryExpiryCount {
inline constexpr AttributeId Id = 0x00000022;
} // namespace TxDirectMaxRetryExpiryCount

namespace TxIndirectMaxRetryExpiryCount {
inline constexpr AttributeId Id = 0x00000023;
} // namespace TxIndirectMaxRetryExpiryCount

namespace TxErrCcaCount {
inline constexpr AttributeId Id = 0x00000024;
} // namespace TxErrCcaCount

namespace TxErrAbortCount {
inline constexpr AttributeId Id = 0x00000025;
} // namespace TxErrAbortCount

namespace TxErrBusyChannelCount {
inline constexpr AttributeId Id = 0x00000026;
} // namespace TxErrBusyChannelCount

namespace RxTotalCount {
inline constexpr AttributeId Id = 0x00000027;
} // namespace RxTotalCount

namespace RxUnicastCount {
inline constexpr AttributeId Id = 0x00000028;
} // namespace RxUnicastCount

namespace RxBroadcastCount {
inline constexpr AttributeId Id = 0x00000029;
} // namespace RxBroadcastCount

namespace RxDataCount {
inline constexpr AttributeId Id = 0x0000002A;
} // namespace RxDataCount

namespace RxDataPollCount {
inline constexpr AttributeId Id = 0x0000002B;
} // namespace RxDataPollCount

namespace RxBeaconCount {
inline constexpr AttributeId Id = 0x0000002C;
} // namespace RxBeaconCount

namespace RxBeaconRequestCount {
inline constexpr AttributeId Id = 0x0000002D;
} // namespace RxBeaconRequestCount

namespace RxOtherCount {
inline constexpr AttributeId Id = 0x0000002E;
} // namespace RxOtherCount

namespace RxAddressFilteredCount {
inline constexpr AttributeId Id = 0x0000002F;
} // namespace RxAddressFilteredCount

namespace RxDestAddrFilteredCount {
inline constexpr AttributeId Id = 0x00000030;
} // namespace RxDestAddrFilteredCount

namespace RxDuplicatedCount {
inline constexpr AttributeId Id = 0x00000031;
} // namespace RxDuplicatedCount

namespace RxErrNoFrameCount {
inline constexpr AttributeId Id = 0x00000032;
} // namespace RxErrNoFrameCount

namespace RxErrUnknownNeighborCount {
inline constexpr AttributeId Id = 0x00000033;
} // namespace RxErrUnknownNeighborCount

namespace RxErrInvalidSrcAddrCount {
inline constexpr AttributeId Id = 0x00000034;
} // namespace RxErrInvalidSrcAddrCount

namespace RxErrSecCount {
inline constexpr AttributeId Id = 0x00000035;
} // namespace RxErrSecCount

namespace RxErrFcsCount {
inline constexpr AttributeId Id = 0x00000036;
} // namespace RxErrFcsCount

namespace RxErrOtherCount {
inline constexpr AttributeId Id = 0x00000037;
} // namespace RxErrOtherCount

namespace ActiveTimestamp {
inline constexpr AttributeId Id = 0x00000038;
} // namespace ActiveTimestamp

namespace PendingTimestamp {
inline constexpr AttributeId Id = 0x00000039;
} // namespace PendingTimestamp

namespace Delay {
inline constexpr AttributeId Id = 0x0000003A;
} // namespace Delay

namespace SecurityPolicy {
inline constexpr AttributeId Id = 0x0000003B;
} // namespace SecurityPolicy

namespace ChannelPage0Mask {
inline constexpr AttributeId Id = 0x0000003C;
} // namespace ChannelPage0Mask

namespace OperationalDatasetComponents {
inline constexpr AttributeId Id = 0x0000003D;
} // namespace OperationalDatasetComponents

namespace ActiveNetworkFaultsList {
inline constexpr AttributeId Id = 0x0000003E;
} // namespace ActiveNetworkFaultsList

namespace ExtAddress {
inline constexpr AttributeId Id = 0x0000003F;
} // namespace ExtAddress

namespace Rloc16 {
inline constexpr AttributeId Id = 0x00000040;
} // namespace Rloc16

namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList

namespace AttributeList {
inline constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList

namespace FeatureMap {
inline constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap

namespace ClusterRevision {
inline constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

} // namespace Attributes
} // namespace ThreadNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
