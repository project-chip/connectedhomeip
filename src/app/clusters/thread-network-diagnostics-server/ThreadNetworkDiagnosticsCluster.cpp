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
    AttributeListBuilder attributeListBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mOptionalAttributes.test(DetachedRoleCount::Id) && mFeatures.Has(Feature::kMLECounts),
          DetachedRoleCount::kMetadataEntry },
        { mOptionalAttributes.test(ChildRoleCount::Id) && mFeatures.Has(Feature::kMLECounts), ChildRoleCount::kMetadataEntry },
        { mOptionalAttributes.test(RouterRoleCount::Id) && mFeatures.Has(Feature::kMLECounts), RouterRoleCount::kMetadataEntry },
        { mOptionalAttributes.test(LeaderRoleCount::Id) && mFeatures.Has(Feature::kMLECounts), LeaderRoleCount::kMetadataEntry },
        { mOptionalAttributes.test(AttachAttemptCount::Id) && mFeatures.Has(Feature::kMLECounts),
          AttachAttemptCount::kMetadataEntry },
        { mOptionalAttributes.test(PartitionIdChangeCount::Id) && mFeatures.Has(Feature::kMLECounts),
          PartitionIdChangeCount::kMetadataEntry },
        { mOptionalAttributes.test(BetterPartitionAttachAttemptCount::Id) && mFeatures.Has(Feature::kMLECounts),
          BetterPartitionAttachAttemptCount::kMetadataEntry },
        { mOptionalAttributes.test(ParentChangeCount::Id) && mFeatures.Has(Feature::kMLECounts),
          ParentChangeCount::kMetadataEntry },
        { mOptionalAttributes.test(TxTotalCount::Id) && mFeatures.Has(Feature::kMACCounts), TxTotalCount::kMetadataEntry },
        { mOptionalAttributes.test(TxUnicastCount::Id) && mFeatures.Has(Feature::kMACCounts), TxUnicastCount::kMetadataEntry },
        { mOptionalAttributes.test(TxBroadcastCount::Id) && mFeatures.Has(Feature::kMACCounts), TxBroadcastCount::kMetadataEntry },
        { mOptionalAttributes.test(TxAckRequestedCount::Id) && mFeatures.Has(Feature::kMACCounts),
          TxAckRequestedCount::kMetadataEntry },
        { mOptionalAttributes.test(TxAckedCount::Id) && mFeatures.Has(Feature::kMACCounts), TxAckedCount::kMetadataEntry },
        { mOptionalAttributes.test(TxNoAckRequestedCount::Id) && mFeatures.Has(Feature::kMACCounts),
          TxNoAckRequestedCount::kMetadataEntry },
        { mOptionalAttributes.test(TxDataCount::Id) && mFeatures.Has(Feature::kMACCounts), TxDataCount::kMetadataEntry },
        { mOptionalAttributes.test(TxDataPollCount::Id) && mFeatures.Has(Feature::kMACCounts), TxDataPollCount::kMetadataEntry },
        { mOptionalAttributes.test(TxBeaconCount::Id) && mFeatures.Has(Feature::kMACCounts), TxBeaconCount::kMetadataEntry },
        { mOptionalAttributes.test(TxBeaconRequestCount::Id) && mFeatures.Has(Feature::kMACCounts),
          TxBeaconRequestCount::kMetadataEntry },
        { mOptionalAttributes.test(TxOtherCount::Id) && mFeatures.Has(Feature::kMACCounts), TxOtherCount::kMetadataEntry },
        { mOptionalAttributes.test(TxRetryCount::Id) && mFeatures.Has(Feature::kMACCounts), TxRetryCount::kMetadataEntry },
        { mOptionalAttributes.test(TxDirectMaxRetryExpiryCount::Id) && mFeatures.Has(Feature::kMACCounts),
          TxDirectMaxRetryExpiryCount::kMetadataEntry },
        { mOptionalAttributes.test(TxIndirectMaxRetryExpiryCount::Id) && mFeatures.Has(Feature::kMACCounts),
          TxIndirectMaxRetryExpiryCount::kMetadataEntry },
        { mOptionalAttributes.test(TxErrCcaCount::Id) && mFeatures.Has(Feature::kMACCounts), TxErrCcaCount::kMetadataEntry },
        { mOptionalAttributes.test(TxErrAbortCount::Id) && mFeatures.Has(Feature::kMACCounts), TxErrAbortCount::kMetadataEntry },
        { mOptionalAttributes.test(TxErrBusyChannelCount::Id) && mFeatures.Has(Feature::kMACCounts),
          TxErrBusyChannelCount::kMetadataEntry },
        { mOptionalAttributes.test(RxTotalCount::Id) && mFeatures.Has(Feature::kMACCounts), RxTotalCount::kMetadataEntry },
        { mOptionalAttributes.test(RxUnicastCount::Id) && mFeatures.Has(Feature::kMACCounts), RxUnicastCount::kMetadataEntry },
        { mOptionalAttributes.test(RxBroadcastCount::Id) && mFeatures.Has(Feature::kMACCounts), RxBroadcastCount::kMetadataEntry },
        { mOptionalAttributes.test(RxDataCount::Id) && mFeatures.Has(Feature::kMACCounts), RxDataCount::kMetadataEntry },
        { mOptionalAttributes.test(RxDataPollCount::Id) && mFeatures.Has(Feature::kMACCounts), RxDataPollCount::kMetadataEntry },
        { mOptionalAttributes.test(RxBeaconCount::Id) && mFeatures.Has(Feature::kMACCounts), RxBeaconCount::kMetadataEntry },
        { mOptionalAttributes.test(RxBeaconRequestCount::Id) && mFeatures.Has(Feature::kMACCounts),
          RxBeaconRequestCount::kMetadataEntry },
        { mOptionalAttributes.test(RxOtherCount::Id) && mFeatures.Has(Feature::kMACCounts), RxOtherCount::kMetadataEntry },
        { mOptionalAttributes.test(RxAddressFilteredCount::Id) && mFeatures.Has(Feature::kMACCounts),
          RxAddressFilteredCount::kMetadataEntry },
        { mOptionalAttributes.test(RxDestAddrFilteredCount::Id) && mFeatures.Has(Feature::kMACCounts),
          RxDestAddrFilteredCount::kMetadataEntry },
        { mOptionalAttributes.test(RxDuplicatedCount::Id) && mFeatures.Has(Feature::kMACCounts),
          RxDuplicatedCount::kMetadataEntry },
        { mOptionalAttributes.test(RxErrNoFrameCount::Id) && mFeatures.Has(Feature::kMACCounts),
          RxErrNoFrameCount::kMetadataEntry },
        { mOptionalAttributes.test(RxErrUnknownNeighborCount::Id) && mFeatures.Has(Feature::kMACCounts),
          RxErrUnknownNeighborCount::kMetadataEntry },
        { mOptionalAttributes.test(RxErrInvalidSrcAddrCount::Id) && mFeatures.Has(Feature::kMACCounts),
          RxErrInvalidSrcAddrCount::kMetadataEntry },
        { mOptionalAttributes.test(RxErrSecCount::Id) && mFeatures.Has(Feature::kMACCounts), RxErrSecCount::kMetadataEntry },
        { mOptionalAttributes.test(RxErrFcsCount::Id) && mFeatures.Has(Feature::kMACCounts), RxErrFcsCount::kMetadataEntry },
        { mOptionalAttributes.test(RxErrOtherCount::Id) && mFeatures.Has(Feature::kMACCounts), RxErrOtherCount::kMetadataEntry },
        { mOptionalAttributes.test(ActiveTimestamp::Id), ActiveTimestamp::kMetadataEntry },
        { mOptionalAttributes.test(PendingTimestamp::Id), PendingTimestamp::kMetadataEntry },
        { mOptionalAttributes.test(Delay::Id), Delay::kMetadataEntry },
        { mFeatures.Has(Feature::kErrorCounts), OverrunCount::kMetadataEntry }
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
