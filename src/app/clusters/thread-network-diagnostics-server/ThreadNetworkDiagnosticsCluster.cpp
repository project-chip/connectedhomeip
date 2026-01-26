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

ThreadNetworkDiagnosticsCluster::ThreadNetworkDiagnosticsCluster(EndpointId endpointId, const BitFlags<Feature> features,
                                                                 const OptionalAttributes optionalAttributes) :
    DefaultServerCluster({ endpointId, ThreadNetworkDiagnostics::Id }),
    mFeatures(features), mOptionalAttributes(optionalAttributes)
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
    AttributeListBuilder listBuilder(builder);

    constexpr AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { true, ActiveTimestamp::kMetadataEntry },
        { true, PendingTimestamp::kMetadataEntry },
        { true, Delay::kMetadataEntry },
        { true, DetachedRoleCount::kMetadataEntry },
        { true, ChildRoleCount::kMetadataEntry },
        { true, RouterRoleCount::kMetadataEntry },
        { true, LeaderRoleCount::kMetadataEntry },
        { true, AttachAttemptCount::kMetadataEntry },
        { true, PartitionIdChangeCount::kMetadataEntry },
        { true, BetterPartitionAttachAttemptCount::kMetadataEntry },
        { true, ParentChangeCount::kMetadataEntry },
        { true, TxTotalCount::kMetadataEntry },
        { true, TxUnicastCount::kMetadataEntry },
        { true, TxBroadcastCount::kMetadataEntry },
        { true, TxAckRequestedCount::kMetadataEntry },
        { true, TxAckedCount::kMetadataEntry },
        { true, TxNoAckRequestedCount::kMetadataEntry },
        { true, TxDataCount::kMetadataEntry },
        { true, TxDataPollCount::kMetadataEntry },
        { true, TxBeaconCount::kMetadataEntry },
        { true, TxBeaconRequestCount::kMetadataEntry },
        { true, TxOtherCount::kMetadataEntry },
        { true, TxRetryCount::kMetadataEntry },
        { true, TxDirectMaxRetryExpiryCount::kMetadataEntry },
        { true, TxIndirectMaxRetryExpiryCount::kMetadataEntry },
        { true, TxErrCcaCount::kMetadataEntry },
        { true, TxErrAbortCount::kMetadataEntry },
        { true, TxErrBusyChannelCount::kMetadataEntry },
        { true, RxTotalCount::kMetadataEntry },
        { true, RxUnicastCount::kMetadataEntry },
        { true, RxBroadcastCount::kMetadataEntry },
        { true, RxDataCount::kMetadataEntry },
        { true, RxDataPollCount::kMetadataEntry },
        { true, RxBeaconCount::kMetadataEntry },
        { true, RxBeaconRequestCount::kMetadataEntry },
        { true, RxOtherCount::kMetadataEntry },
        { true, RxAddressFilteredCount::kMetadataEntry },
        { true, RxDestAddrFilteredCount::kMetadataEntry },
        { true, RxDuplicatedCount::kMetadataEntry },
        { true, RxErrNoFrameCount::kMetadataEntry },
        { true, RxErrUnknownNeighborCount::kMetadataEntry },
        { true, RxErrInvalidSrcAddrCount::kMetadataEntry },
        { true, RxErrSecCount::kMetadataEntry },
        { true, RxErrFcsCount::kMetadataEntry },
        { true, RxErrOtherCount::kMetadataEntry },
        { true, OverrunCount::kMetadataEntry }
    };

    if (mFeatures.Raw() == 0)
    {
        return listBuilder.Append(Span(ThreadNetworkDiagnostics::Attributes::kMandatoryMetadata), {});
    }

    return listBuilder.Append(Span(ThreadNetworkDiagnostics::Attributes::kMandatoryMetadata), Span(optionalAttributes));
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
    if (mFeatures.Has(Feature::kErrorCounts))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            Commands::ResetCounts::kMetadataEntry,
        }));
    }

    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
