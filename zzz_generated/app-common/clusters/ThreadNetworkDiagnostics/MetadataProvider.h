// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThreadNetworkDiagnostics (cluster code: 53/0x35)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ThreadNetworkDiagnostics/Ids.h>
#include <clusters/ThreadNetworkDiagnostics/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ThreadNetworkDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ThreadNetworkDiagnostics::Attributes;
        switch (attributeId)
        {
        case Channel::Id:
            return Channel::kMetadataEntry;
        case RoutingRole::Id:
            return RoutingRole::kMetadataEntry;
        case NetworkName::Id:
            return NetworkName::kMetadataEntry;
        case PanId::Id:
            return PanId::kMetadataEntry;
        case ExtendedPanId::Id:
            return ExtendedPanId::kMetadataEntry;
        case MeshLocalPrefix::Id:
            return MeshLocalPrefix::kMetadataEntry;
        case OverrunCount::Id:
            return OverrunCount::kMetadataEntry;
        case NeighborTable::Id:
            return NeighborTable::kMetadataEntry;
        case RouteTable::Id:
            return RouteTable::kMetadataEntry;
        case PartitionId::Id:
            return PartitionId::kMetadataEntry;
        case Weighting::Id:
            return Weighting::kMetadataEntry;
        case DataVersion::Id:
            return DataVersion::kMetadataEntry;
        case StableDataVersion::Id:
            return StableDataVersion::kMetadataEntry;
        case LeaderRouterId::Id:
            return LeaderRouterId::kMetadataEntry;
        case DetachedRoleCount::Id:
            return DetachedRoleCount::kMetadataEntry;
        case ChildRoleCount::Id:
            return ChildRoleCount::kMetadataEntry;
        case RouterRoleCount::Id:
            return RouterRoleCount::kMetadataEntry;
        case LeaderRoleCount::Id:
            return LeaderRoleCount::kMetadataEntry;
        case AttachAttemptCount::Id:
            return AttachAttemptCount::kMetadataEntry;
        case PartitionIdChangeCount::Id:
            return PartitionIdChangeCount::kMetadataEntry;
        case BetterPartitionAttachAttemptCount::Id:
            return BetterPartitionAttachAttemptCount::kMetadataEntry;
        case ParentChangeCount::Id:
            return ParentChangeCount::kMetadataEntry;
        case TxTotalCount::Id:
            return TxTotalCount::kMetadataEntry;
        case TxUnicastCount::Id:
            return TxUnicastCount::kMetadataEntry;
        case TxBroadcastCount::Id:
            return TxBroadcastCount::kMetadataEntry;
        case TxAckRequestedCount::Id:
            return TxAckRequestedCount::kMetadataEntry;
        case TxAckedCount::Id:
            return TxAckedCount::kMetadataEntry;
        case TxNoAckRequestedCount::Id:
            return TxNoAckRequestedCount::kMetadataEntry;
        case TxDataCount::Id:
            return TxDataCount::kMetadataEntry;
        case TxDataPollCount::Id:
            return TxDataPollCount::kMetadataEntry;
        case TxBeaconCount::Id:
            return TxBeaconCount::kMetadataEntry;
        case TxBeaconRequestCount::Id:
            return TxBeaconRequestCount::kMetadataEntry;
        case TxOtherCount::Id:
            return TxOtherCount::kMetadataEntry;
        case TxRetryCount::Id:
            return TxRetryCount::kMetadataEntry;
        case TxDirectMaxRetryExpiryCount::Id:
            return TxDirectMaxRetryExpiryCount::kMetadataEntry;
        case TxIndirectMaxRetryExpiryCount::Id:
            return TxIndirectMaxRetryExpiryCount::kMetadataEntry;
        case TxErrCcaCount::Id:
            return TxErrCcaCount::kMetadataEntry;
        case TxErrAbortCount::Id:
            return TxErrAbortCount::kMetadataEntry;
        case TxErrBusyChannelCount::Id:
            return TxErrBusyChannelCount::kMetadataEntry;
        case RxTotalCount::Id:
            return RxTotalCount::kMetadataEntry;
        case RxUnicastCount::Id:
            return RxUnicastCount::kMetadataEntry;
        case RxBroadcastCount::Id:
            return RxBroadcastCount::kMetadataEntry;
        case RxDataCount::Id:
            return RxDataCount::kMetadataEntry;
        case RxDataPollCount::Id:
            return RxDataPollCount::kMetadataEntry;
        case RxBeaconCount::Id:
            return RxBeaconCount::kMetadataEntry;
        case RxBeaconRequestCount::Id:
            return RxBeaconRequestCount::kMetadataEntry;
        case RxOtherCount::Id:
            return RxOtherCount::kMetadataEntry;
        case RxAddressFilteredCount::Id:
            return RxAddressFilteredCount::kMetadataEntry;
        case RxDestAddrFilteredCount::Id:
            return RxDestAddrFilteredCount::kMetadataEntry;
        case RxDuplicatedCount::Id:
            return RxDuplicatedCount::kMetadataEntry;
        case RxErrNoFrameCount::Id:
            return RxErrNoFrameCount::kMetadataEntry;
        case RxErrUnknownNeighborCount::Id:
            return RxErrUnknownNeighborCount::kMetadataEntry;
        case RxErrInvalidSrcAddrCount::Id:
            return RxErrInvalidSrcAddrCount::kMetadataEntry;
        case RxErrSecCount::Id:
            return RxErrSecCount::kMetadataEntry;
        case RxErrFcsCount::Id:
            return RxErrFcsCount::kMetadataEntry;
        case RxErrOtherCount::Id:
            return RxErrOtherCount::kMetadataEntry;
        case ActiveTimestamp::Id:
            return ActiveTimestamp::kMetadataEntry;
        case PendingTimestamp::Id:
            return PendingTimestamp::kMetadataEntry;
        case Delay::Id:
            return Delay::kMetadataEntry;
        case SecurityPolicy::Id:
            return SecurityPolicy::kMetadataEntry;
        case ChannelPage0Mask::Id:
            return ChannelPage0Mask::kMetadataEntry;
        case OperationalDatasetComponents::Id:
            return OperationalDatasetComponents::kMetadataEntry;
        case ActiveNetworkFaultsList::Id:
            return ActiveNetworkFaultsList::kMetadataEntry;
        case ExtAddress::Id:
            return ExtAddress::kMetadataEntry;
        case Rloc16::Id:
            return Rloc16::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ThreadNetworkDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ThreadNetworkDiagnostics::Commands;
        switch (commandId)
        {
        case ResetCounts::Id:
            return ResetCounts::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
