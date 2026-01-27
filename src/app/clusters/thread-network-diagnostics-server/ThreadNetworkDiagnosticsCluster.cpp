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

using namespace ThreadNetworkDiagnostics;
using namespace ThreadNetworkDiagnostics::Attributes;

ThreadNetworkDiagnosticsCluster::ThreadNetworkDiagnosticsCluster(EndpointId endpointId, ClusterType clusterType) :
    DefaultServerCluster({ endpointId, ThreadNetworkDiagnostics::Id }), mClusterType(clusterType)
{}

DataModel::ActionReturnStatus ThreadNetworkDiagnosticsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                             AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(ThreadNetworkDiagnostics::kRevision);
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(mClusterType == ClusterType::kMinimal ? 0 : 0xF);
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
    AttributeListBuilder listBuilder(builder);

    constexpr DataModel::AttributeEntry fullAttributes[] = {
        // Mandatory Attributes
        Attributes::Channel::kMetadataEntry,          //
        RoutingRole::kMetadataEntry,                  //
        NetworkName::kMetadataEntry,                  //
        PanId::kMetadataEntry,                        //
        ExtendedPanId::kMetadataEntry,                //
        MeshLocalPrefix::kMetadataEntry,              //
        NeighborTable::kMetadataEntry,                //
        RouteTable::kMetadataEntry,                   //
        PartitionId::kMetadataEntry,                  //
        Weighting::kMetadataEntry,                    //
        Attributes::DataVersion::kMetadataEntry,      //
        StableDataVersion::kMetadataEntry,            //
        LeaderRouterId::kMetadataEntry,               //
        SecurityPolicy::kMetadataEntry,               //
        ChannelPage0Mask::kMetadataEntry,             //
        OperationalDatasetComponents::kMetadataEntry, //
        ActiveNetworkFaultsList::kMetadataEntry,      //
        ExtAddress::kMetadataEntry,                   //
        Rloc16::kMetadataEntry,                       //
        // Optional Attributes
        ActiveTimestamp::kMetadataEntry,                   //
        PendingTimestamp::kMetadataEntry,                  //
        Delay::kMetadataEntry,                             //
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
        OverrunCount::kMetadataEntry                       //
    };

    if (mClusterType == ClusterType::kMinimal)
    {
        return listBuilder.Append(Span(ThreadNetworkDiagnostics::Attributes::kMandatoryMetadata), {});
    }
    return listBuilder.Append(Span(fullAttributes), {});
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
    if (mClusterType == ClusterType::kFull)
    {
        ReturnErrorOnFailure(builder.AppendElements({
            Commands::ResetCounts::kMetadataEntry,
        }));
    }

    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
