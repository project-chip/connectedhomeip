/**
 *
 *    Copyright (c) 2021-2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <app/clusters/thread-network-diagnostics-server/ThreadNetworkDiagnosticsCluster.h>
#include <app/clusters/thread-network-diagnostics-server/ThreadNetworkDiagnosticsProvider.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ThreadNetworkDiagnostics/Metadata.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThreadNetworkDiagnostics;
using namespace chip::app::Clusters::ThreadNetworkDiagnostics::Attributes;
using namespace chip::DeviceLayer;

namespace chip::app::Clusters {

using namespace ThreadNetworkDiagnostics::Attributes;

ThreadNetworkDiagnosticsCluster::ThreadNetworkDiagnosticsCluster(EndpointId endpointId,
                                                                 const BitFlags<ThreadNetworkDiagnostics::Feature> features,
                                                                 const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, ThreadNetworkDiagnostics::Id }), mFeatures(features), mConfig(config)
{}

DataModel::ActionReturnStatus ThreadNetworkDiagnosticsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                             AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(ThreadNetworkDiagnostics::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case NeighborTable::Id:
    case RouteTable::Id:
    case SecurityPolicy::Id:
    case OperationalDatasetComponents::Id:
    case ActiveNetworkFaultsList::Id:
    case Attributes::Channel::Id:
    case RoutingRole::Id:
    case NetworkName::Id:
    case PanId::Id:
    case ExtendedPanId::Id:
    case MeshLocalPrefix::Id:
    case PartitionId::Id:
    case Weighting::Id:
    case DataVersion::Id:
    case StableDataVersion::Id:
    case LeaderRouterId::Id:
    case OverrunCount::Id:
    case DetachedRoleCount::Id:
    case ChildRoleCount::Id:
    case RouterRoleCount::Id:
    case LeaderRoleCount::Id:
    case AttachAttemptCount::Id:
    case PartitionIdChangeCount::Id:
    case BetterPartitionAttachAttemptCount::Id:
    case ParentChangeCount::Id:
    case TxTotalCount::Id:
    case TxUnicastCount::Id:
    case TxBroadcastCount::Id:
    case TxAckRequestedCount::Id:
    case TxAckedCount::Id:
    case TxNoAckRequestedCount::Id:
    case TxDataCount::Id:
    case TxDataPollCount::Id:
    case TxBeaconCount::Id:
    case TxBeaconRequestCount::Id:
    case TxOtherCount::Id:
    case TxRetryCount::Id:
    case TxDirectMaxRetryExpiryCount::Id:
    case TxIndirectMaxRetryExpiryCount::Id:
    case TxErrCcaCount::Id:
    case TxErrAbortCount::Id:
    case TxErrBusyChannelCount::Id:
    case RxTotalCount::Id:
    case RxUnicastCount::Id:
    case RxBroadcastCount::Id:
    case RxDataCount::Id:
    case RxDataPollCount::Id:
    case RxBeaconCount::Id:
    case RxBeaconRequestCount::Id:
    case RxOtherCount::Id:
    case RxAddressFilteredCount::Id:
    case RxDestAddrFilteredCount::Id:
    case RxDuplicatedCount::Id:
    case RxErrNoFrameCount::Id:
    case RxErrUnknownNeighborCount::Id:
    case RxErrInvalidSrcAddrCount::Id:
    case RxErrSecCount::Id:
    case RxErrFcsCount::Id:
    case RxErrOtherCount::Id:
    case ActiveTimestamp::Id:
    case PendingTimestamp::Id:
    case Delay::Id:
    case ChannelPage0Mask::Id:
    case ExtAddress::Id:
    case Rloc16::Id:
        return WriteThreadNetworkDiagnosticAttributeToTlv(request.path.mAttributeId, encoder);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ThreadNetworkDiagnosticsCluster::Attributes(const ConcreteClusterPath & path,
                                                       ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder attributeListBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mConfig.detachedRoleCount && mFeatures.Has(Feature::kMLECounts), DetachedRoleCount::kMetadataEntry },
        { mConfig.childRoleCount && mFeatures.Has(Feature::kMLECounts), ChildRoleCount::kMetadataEntry },
        { mConfig.routerRoleCount && mFeatures.Has(Feature::kMLECounts), RouterRoleCount::kMetadataEntry },
        { mConfig.leaderRoleCount && mFeatures.Has(Feature::kMLECounts), LeaderRoleCount::kMetadataEntry },
        { mConfig.attachAttemptCount && mFeatures.Has(Feature::kMLECounts), AttachAttemptCount::kMetadataEntry },
        { mConfig.partitionIdChangeCount && mFeatures.Has(Feature::kMLECounts), PartitionIdChangeCount::kMetadataEntry },
        { mConfig.betterPartitionAttachAttemptCount && mFeatures.Has(Feature::kMLECounts),
          BetterPartitionAttachAttemptCount::kMetadataEntry },
        { mConfig.parentChangeCount && mFeatures.Has(Feature::kMLECounts), ParentChangeCount::kMetadataEntry },
        { mConfig.txTotalCount && mFeatures.Has(Feature::kMACCounts), TxTotalCount::kMetadataEntry },
        { mConfig.txUnicastCount && mFeatures.Has(Feature::kMACCounts), TxUnicastCount::kMetadataEntry },
        { mConfig.txBroadcastCount && mFeatures.Has(Feature::kMACCounts), TxBroadcastCount::kMetadataEntry },
        { mConfig.txAckRequestedCount && mFeatures.Has(Feature::kMACCounts), TxAckRequestedCount::kMetadataEntry },
        { mConfig.txAckedCount && mFeatures.Has(Feature::kMACCounts), TxAckedCount::kMetadataEntry },
        { mConfig.txNoAckRequestedCount && mFeatures.Has(Feature::kMACCounts), TxNoAckRequestedCount::kMetadataEntry },
        { mConfig.txDataCount && mFeatures.Has(Feature::kMACCounts), TxDataCount::kMetadataEntry },
        { mConfig.txDataPollCount && mFeatures.Has(Feature::kMACCounts), TxDataPollCount::kMetadataEntry },
        { mConfig.txBeaconCount && mFeatures.Has(Feature::kMACCounts), TxBeaconCount::kMetadataEntry },
        { mConfig.txBeaconRequestCount && mFeatures.Has(Feature::kMACCounts), TxBeaconRequestCount::kMetadataEntry },
        { mConfig.txOtherCount && mFeatures.Has(Feature::kMACCounts), TxOtherCount::kMetadataEntry },
        { mConfig.txRetryCount && mFeatures.Has(Feature::kMACCounts), TxRetryCount::kMetadataEntry },
        { mConfig.txDirectMaxRetryExpiryCount && mFeatures.Has(Feature::kMACCounts), TxDirectMaxRetryExpiryCount::kMetadataEntry },
        { mConfig.txIndirectMaxRetryExpiryCount && mFeatures.Has(Feature::kMACCounts),
          TxIndirectMaxRetryExpiryCount::kMetadataEntry },
        { mConfig.txErrCcaCount && mFeatures.Has(Feature::kMACCounts), TxErrCcaCount::kMetadataEntry },
        { mConfig.txErrAbortCount && mFeatures.Has(Feature::kMACCounts), TxErrAbortCount::kMetadataEntry },
        { mConfig.txErrBusyChannelCount && mFeatures.Has(Feature::kMACCounts), TxErrBusyChannelCount::kMetadataEntry },
        { mConfig.rxTotalCount && mFeatures.Has(Feature::kMACCounts), RxTotalCount::kMetadataEntry },
        { mConfig.rxUnicastCount && mFeatures.Has(Feature::kMACCounts), RxUnicastCount::kMetadataEntry },
        { mConfig.rxBroadcastCount && mFeatures.Has(Feature::kMACCounts), RxBroadcastCount::kMetadataEntry },
        { mConfig.rxDataCount && mFeatures.Has(Feature::kMACCounts), RxDataCount::kMetadataEntry },
        { mConfig.rxDataPollCount && mFeatures.Has(Feature::kMACCounts), RxDataPollCount::kMetadataEntry },
        { mConfig.rxBeaconCount && mFeatures.Has(Feature::kMACCounts), RxBeaconCount::kMetadataEntry },
        { mConfig.rxBeaconRequestCount && mFeatures.Has(Feature::kMACCounts), RxBeaconRequestCount::kMetadataEntry },
        { mConfig.rxOtherCount && mFeatures.Has(Feature::kMACCounts), RxOtherCount::kMetadataEntry },
        { mConfig.rxAddressFilteredCount && mFeatures.Has(Feature::kMACCounts), RxAddressFilteredCount::kMetadataEntry },
        { mConfig.rxDestAddrFilteredCount && mFeatures.Has(Feature::kMACCounts), RxDestAddrFilteredCount::kMetadataEntry },
        { mConfig.rxDuplicatedCount && mFeatures.Has(Feature::kMACCounts), RxDuplicatedCount::kMetadataEntry },
        { mConfig.rxErrNoFrameCount && mFeatures.Has(Feature::kMACCounts), RxErrNoFrameCount::kMetadataEntry },
        { mConfig.rxErrUnknownNeighborCount && mFeatures.Has(Feature::kMACCounts), RxErrUnknownNeighborCount::kMetadataEntry },
        { mConfig.rxErrInvalidSrcAddrCount && mFeatures.Has(Feature::kMACCounts), RxErrInvalidSrcAddrCount::kMetadataEntry },
        { mConfig.rxErrSecCount && mFeatures.Has(Feature::kMACCounts), RxErrSecCount::kMetadataEntry },
        { mConfig.rxErrFcsCount && mFeatures.Has(Feature::kMACCounts), RxErrFcsCount::kMetadataEntry },
        { mConfig.rxErrOtherCount && mFeatures.Has(Feature::kMACCounts), RxErrOtherCount::kMetadataEntry },
        { mFeatures.Has(Feature::kErrorCounts), OverrunCount::kMetadataEntry },
        { mConfig.activeTs, ActiveTimestamp::kMetadataEntry },
        { mConfig.pendingTs, PendingTimestamp::kMetadataEntry },
        { mConfig.delay, Delay::kMetadataEntry }
    };

    return attributeListBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

// Notified when the Node’s connection status to a Thread network has changed.
void ThreadNetworkDiagnosticsCluster::OnConnectionStatusChanged(ConnectionStatusEnum newConnectionStatus)
{
    ChipLogProgress(Zcl, "ThreadNetworkDiagnosticsCluster: OnConnectionStatusChanged");

    VerifyOrReturn(mContext != nullptr);
    Events::ConnectionStatus::Type event{ newConnectionStatus };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

// Notified when the Node’s faults related to a Thread network have changed.
void ThreadNetworkDiagnosticsCluster::OnNetworkFaultChanged(const GeneralFaults<kMaxNetworkFaults> & previous,
                                                            const GeneralFaults<kMaxNetworkFaults> & current)
{
    ChipLogProgress(Zcl, "ThreadNetworkDiagnosticsCluster: OnNetworkFaultChanged");

    /* Verify that the data size matches the expected one. */
    static_assert(sizeof(*current.data()) == sizeof(NetworkFaultEnum));

    DataModel::List<const NetworkFaultEnum> currentList(reinterpret_cast<const NetworkFaultEnum *>(current.data()), current.size());
    DataModel::List<const NetworkFaultEnum> previousList(reinterpret_cast<const NetworkFaultEnum *>(previous.data()),
                                                         previous.size());

    VerifyOrReturn(mContext != nullptr);
    Events::NetworkFaultChange::Type event{ currentList, previousList };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

CHIP_ERROR ThreadNetworkDiagnosticsCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    GetDiagnosticDataProvider().SetThreadDiagnosticsDelegate(this);
    return CHIP_NO_ERROR;
}

void ThreadNetworkDiagnosticsCluster::Shutdown(ClusterShutdownType type)
{
    GetDiagnosticDataProvider().SetThreadDiagnosticsDelegate(nullptr);
    DefaultServerCluster::Shutdown(type);
}

std::optional<DataModel::ActionReturnStatus>
ThreadNetworkDiagnosticsCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                               CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::ResetCounts::Id: {
        Commands::ResetCounts::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        ConnectivityMgr().ResetThreadNetworkDiagnosticsCounts();
        return Protocols::InteractionModel::Status::Success;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR ThreadNetworkDiagnosticsCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                             ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mFeatures.Has(Feature::kErrorCounts))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            Commands::ResetCounts::kMetadataEntry,
        }));
    }

    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
