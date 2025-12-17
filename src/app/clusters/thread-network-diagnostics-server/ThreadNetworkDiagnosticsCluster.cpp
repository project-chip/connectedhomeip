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
                                                                 const OptionalAttributeSet & optionalAttributeSet,
                                                                 const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, ThreadNetworkDiagnostics::Id }), mFeatures(features),
    mOptionalAttributeSet(optionalAttributeSet), mActiveTs(config.activeTs), mPendingTs(config.pendingTs), mDelay(config.delay)
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
    case ActiveTimestamp::Id:
        return encoder.Encode(mActiveTs);
    case PendingTimestamp::Id:
        return encoder.Encode(mPendingTs);
    case Delay::Id:
        return encoder.Encode(mDelay);
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

    const DataModel::AttributeEntry optionalAttributes[] = {
        DetachedRoleCount::kMetadataEntry,                 //
        ChildRoleCount::kMetadataEntry,                    //
        RouterRoleCount::kMetadataEntry,                   //
        LeaderRoleCount::kMetadataEntry,                   //
        AttachAttemptCount::kMetadataEntry,                //
        PartitionIdChangeCount::kMetadataEntry,            //
        BetterPartitionAttachAttemptCount::kMetadataEntry, //
        ParentChangeCount::kMetadataEntry,                 //
        TxTotalCount::kMetadataEntry,                      //
        TxUnicastCount::kMetadataEntry,                    //
        TxBroadcastCount::kMetadataEntry,                  //
        TxAckRequestedCount::kMetadataEntry,               //
        TxAckedCount::kMetadataEntry,                      //
        TxNoAckRequestedCount::kMetadataEntry,             //
        TxDataCount::kMetadataEntry,                       //
        TxDataPollCount::kMetadataEntry,                   //
        TxBeaconCount::kMetadataEntry,                     //
        TxBeaconRequestCount::kMetadataEntry,              //
        TxOtherCount::kMetadataEntry,                      //
        TxRetryCount::kMetadataEntry,                      //
        TxDirectMaxRetryExpiryCount::kMetadataEntry,       //
        TxIndirectMaxRetryExpiryCount::kMetadataEntry,     //
        TxErrCcaCount::kMetadataEntry,                     //
        TxErrAbortCount::kMetadataEntry,                   //
        TxErrBusyChannelCount::kMetadataEntry,             //
        RxTotalCount::kMetadataEntry,                      //
        RxUnicastCount::kMetadataEntry,                    //
        RxBroadcastCount::kMetadataEntry,                  //
        RxDataCount::kMetadataEntry,                       //
        RxDataPollCount::kMetadataEntry,                   //
        RxBeaconCount::kMetadataEntry,                     //
        RxBeaconRequestCount::kMetadataEntry,              //
        RxOtherCount::kMetadataEntry,                      //
        RxAddressFilteredCount::kMetadataEntry,            //
        RxDestAddrFilteredCount::kMetadataEntry,           //
        RxDuplicatedCount::kMetadataEntry,                 //
        RxErrNoFrameCount::kMetadataEntry,                 //
        RxErrUnknownNeighborCount::kMetadataEntry,         //
        RxErrInvalidSrcAddrCount::kMetadataEntry,          //
        RxErrSecCount::kMetadataEntry,                     //
        RxErrFcsCount::kMetadataEntry,                     //
        RxErrOtherCount::kMetadataEntry,                   //
        OverrunCount::kMetadataEntry,                      //
        ActiveTimestamp::kMetadataEntry,                   //
        PendingTimestamp::kMetadataEntry,                  //
        Delay::kMetadataEntry                              //
    };

    // Full attribute set, to combine real "optional" attributes but also
    // attributes controlled by feature flags.
    OptionalAttributeSet64<DetachedRoleCount::Id,                 //
                           ChildRoleCount::Id,                    //
                           RouterRoleCount::Id,                   //
                           LeaderRoleCount::Id,                   //
                           AttachAttemptCount::Id,                //
                           PartitionIdChangeCount::Id,            //
                           BetterPartitionAttachAttemptCount::Id, //
                           ParentChangeCount::Id,                 //
                           TxTotalCount::Id,                      //
                           TxUnicastCount::Id,                    //
                           TxBroadcastCount::Id,                  //
                           TxAckRequestedCount::Id,               //
                           TxAckedCount::Id,                      //
                           TxNoAckRequestedCount::Id,             //
                           TxDataCount::Id,                       //
                           TxDataPollCount::Id,                   //
                           TxBeaconCount::Id,                     //
                           TxBeaconRequestCount::Id,              //
                           TxOtherCount::Id,                      //
                           TxRetryCount::Id,                      //
                           TxDirectMaxRetryExpiryCount::Id,       //
                           TxIndirectMaxRetryExpiryCount::Id,     //
                           TxErrCcaCount::Id,                     //
                           TxErrAbortCount::Id,                   //
                           TxErrBusyChannelCount::Id,             //
                           RxTotalCount::Id,                      //
                           RxUnicastCount::Id,                    //
                           RxBroadcastCount::Id,                  //
                           RxDataCount::Id,                       //
                           RxDataPollCount::Id,                   //
                           RxBeaconCount::Id,                     //
                           RxBeaconRequestCount::Id,              //
                           RxOtherCount::Id,                      //
                           RxAddressFilteredCount::Id,            //
                           RxDestAddrFilteredCount::Id,           //
                           RxDuplicatedCount::Id,                 //
                           RxErrNoFrameCount::Id,                 //
                           RxErrUnknownNeighborCount::Id,         //
                           RxErrInvalidSrcAddrCount::Id,          //
                           RxErrSecCount::Id,                     //
                           RxErrFcsCount::Id,                     //
                           RxErrOtherCount::Id,                   //
                           OverrunCount::Id,                      //
                           ActiveTimestamp::Id,                   //
                           PendingTimestamp::Id,                  //
                           Delay::Id                              //
                           >
        optionalAttributeSet(mOptionalAttributeSet);

    if (mFeatures.Has(Feature::kMLECounts))
    {
        optionalAttributeSet.Set<Attributes::DetachedRoleCount::Id>();
        optionalAttributeSet.Set<ChildRoleCount::Id>();
        optionalAttributeSet.Set<RouterRoleCount::Id>();
        optionalAttributeSet.Set<LeaderRoleCount::Id>();
        optionalAttributeSet.Set<AttachAttemptCount::Id>();
        optionalAttributeSet.Set<PartitionIdChangeCount::Id>();
        optionalAttributeSet.Set<BetterPartitionAttachAttemptCount::Id>();
        optionalAttributeSet.Set<ParentChangeCount::Id>();
    }

    if (mFeatures.Has(Feature::kMACCounts))
    {
        optionalAttributeSet.Set<TxTotalCount::Id>();
        optionalAttributeSet.Set<TxUnicastCount::Id>();
        optionalAttributeSet.Set<TxBroadcastCount::Id>();
        optionalAttributeSet.Set<TxAckRequestedCount::Id>();
        optionalAttributeSet.Set<TxAckedCount::Id>();
        optionalAttributeSet.Set<TxNoAckRequestedCount::Id>();
        optionalAttributeSet.Set<TxDataCount::Id>();
        optionalAttributeSet.Set<TxDataPollCount::Id>();
        optionalAttributeSet.Set<TxBeaconCount::Id>();
        optionalAttributeSet.Set<TxBeaconRequestCount::Id>();
        optionalAttributeSet.Set<TxOtherCount::Id>();
        optionalAttributeSet.Set<TxRetryCount::Id>();
        optionalAttributeSet.Set<TxDirectMaxRetryExpiryCount::Id>();
        optionalAttributeSet.Set<TxIndirectMaxRetryExpiryCount::Id>();
        optionalAttributeSet.Set<TxErrCcaCount::Id>();
        optionalAttributeSet.Set<TxErrAbortCount::Id>();
        optionalAttributeSet.Set<TxErrBusyChannelCount::Id>();
        optionalAttributeSet.Set<RxTotalCount::Id>();
        optionalAttributeSet.Set<RxUnicastCount::Id>();
        optionalAttributeSet.Set<RxBroadcastCount::Id>();
        optionalAttributeSet.Set<RxDataCount::Id>();
        optionalAttributeSet.Set<RxDataPollCount::Id>();
        optionalAttributeSet.Set<RxBeaconCount::Id>();
        optionalAttributeSet.Set<RxBeaconRequestCount::Id>();
        optionalAttributeSet.Set<RxOtherCount::Id>();
        optionalAttributeSet.Set<RxAddressFilteredCount::Id>();
        optionalAttributeSet.Set<RxDestAddrFilteredCount::Id>();
        optionalAttributeSet.Set<RxDuplicatedCount::Id>();
        optionalAttributeSet.Set<RxErrNoFrameCount::Id>();
        optionalAttributeSet.Set<RxErrUnknownNeighborCount::Id>();
        optionalAttributeSet.Set<RxErrInvalidSrcAddrCount::Id>();
        optionalAttributeSet.Set<RxErrSecCount::Id>();
        optionalAttributeSet.Set<RxErrFcsCount::Id>();
        optionalAttributeSet.Set<RxErrOtherCount::Id>();
    }

    if (mFeatures.Has(Feature::kErrorCounts))
    {
        optionalAttributeSet.Set<OverrunCount::Id>();
    }

    return attributeListBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet);
}

// Notified when the Node’s connection status to a Thread network has changed.
void ThreadNetworkDiagnosticsCluster::OnConnectionStatusChanged(ConnectionStatusEnum newConnectionStatus)
{
    ChipLogProgress(Zcl, "ThreadNetworkDiagnosticsCluster: OnConnectionStatusChanged");

    VerifyOrReturn(mContext != nullptr);
    Events::ConnectionStatus::Type event{ newConnectionStatus };
    auto eventNumber = mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    if (eventNumber == std::nullopt)
    {
        ChipLogError(Zcl, "ThreadNetworkDiagnosticsCluster: Failed to record ConnectionStatus event");
    }
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
    auto eventNumber = mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    if (eventNumber == std::nullopt)
    {
        ChipLogError(Zcl, "ThreadNetworkDiagnosticsCluster: Failed to record NetworkFaultChange event");
    }
}

CHIP_ERROR ThreadNetworkDiagnosticsCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    GetDiagnosticDataProvider().SetThreadDiagnosticsDelegate(this);
    return CHIP_NO_ERROR;
}

void ThreadNetworkDiagnosticsCluster::Shutdown()
{
    GetDiagnosticDataProvider().SetThreadDiagnosticsDelegate(nullptr);
    DefaultServerCluster::Shutdown();
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
        return HandleResetCounts(handler, request.path, data);
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

std::optional<DataModel::ActionReturnStatus>
ThreadNetworkDiagnosticsCluster::HandleResetCounts(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                   const Commands::ResetCounts::DecodableType & commandData)
{
    ConnectivityMgr().ResetThreadNetworkDiagnosticsCounts();
    return Protocols::InteractionModel::Status::Success;
}

} // namespace chip::app::Clusters
