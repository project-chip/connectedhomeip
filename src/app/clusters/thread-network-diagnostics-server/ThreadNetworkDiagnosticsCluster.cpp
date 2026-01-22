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
    DefaultServerCluster({ endpointId, ThreadNetworkDiagnostics::Id }), mFeatures(features), mOptionalAttributes(optionalAttributes)
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

namespace {

struct AttributeFeature
{
    AttributeId id;
    std::optional<Feature> feature;
};

constexpr AttributeFeature gAttributes[] = { { ActiveTimestamp::Id, std::nullopt },
                                             { PendingTimestamp::Id, std::nullopt },
                                             { Delay::Id, std::nullopt },
                                             { DetachedRoleCount::Id, Feature::kMLECounts },
                                             { ChildRoleCount::Id, Feature::kMLECounts },
                                             { RouterRoleCount::Id, Feature::kMLECounts },
                                             { LeaderRoleCount::Id, Feature::kMLECounts },
                                             { AttachAttemptCount::Id, Feature::kMLECounts },
                                             { PartitionIdChangeCount::Id, Feature::kMLECounts },
                                             { BetterPartitionAttachAttemptCount::Id, Feature::kMLECounts },
                                             { ParentChangeCount::Id, Feature::kMLECounts },
                                             { TxTotalCount::Id, Feature::kMACCounts },
                                             { TxUnicastCount::Id, Feature::kMACCounts },
                                             { TxBroadcastCount::Id, Feature::kMACCounts },
                                             { TxAckRequestedCount::Id, Feature::kMACCounts },
                                             { TxAckedCount::Id, Feature::kMACCounts },
                                             { TxNoAckRequestedCount::Id, Feature::kMACCounts },
                                             { TxDataCount::Id, Feature::kMACCounts },
                                             { TxDataPollCount::Id, Feature::kMACCounts },
                                             { TxBeaconCount::Id, Feature::kMACCounts },
                                             { TxBeaconRequestCount::Id, Feature::kMACCounts },
                                             { TxOtherCount::Id, Feature::kMACCounts },
                                             { TxRetryCount::Id, Feature::kMACCounts },
                                             { TxDirectMaxRetryExpiryCount::Id, Feature::kMACCounts },
                                             { TxIndirectMaxRetryExpiryCount::Id, Feature::kMACCounts },
                                             { TxErrCcaCount::Id, Feature::kMACCounts },
                                             { TxErrAbortCount::Id, Feature::kMACCounts },
                                             { TxErrBusyChannelCount::Id, Feature::kMACCounts },
                                             { RxTotalCount::Id, Feature::kMACCounts },
                                             { RxUnicastCount::Id, Feature::kMACCounts },
                                             { RxBroadcastCount::Id, Feature::kMACCounts },
                                             { RxDataCount::Id, Feature::kMACCounts },
                                             { RxDataPollCount::Id, Feature::kMACCounts },
                                             { RxBeaconCount::Id, Feature::kMACCounts },
                                             { RxBeaconRequestCount::Id, Feature::kMACCounts },
                                             { RxOtherCount::Id, Feature::kMACCounts },
                                             { RxAddressFilteredCount::Id, Feature::kMACCounts },
                                             { RxDestAddrFilteredCount::Id, Feature::kMACCounts },
                                             { RxDuplicatedCount::Id, Feature::kMACCounts },
                                             { RxErrNoFrameCount::Id, Feature::kMACCounts },
                                             { RxErrUnknownNeighborCount::Id, Feature::kMACCounts },
                                             { RxErrInvalidSrcAddrCount::Id, Feature::kMACCounts },
                                             { RxErrSecCount::Id, Feature::kMACCounts },
                                             { RxErrFcsCount::Id, Feature::kMACCounts },
                                             { RxErrOtherCount::Id, Feature::kMACCounts },
                                             { OverrunCount::Id, Feature::kErrorCounts } };

// The metadata entries in gAttributeEntries[] need to match the same order of AttributeId in gAttributes[]
AttributeListBuilder::OptionalAttributeEntry gAttributeEntries[] = { { false, ActiveTimestamp::kMetadataEntry },
                                                                     { false, PendingTimestamp::kMetadataEntry },
                                                                     { false, Delay::kMetadataEntry },
                                                                     { false, DetachedRoleCount::kMetadataEntry },
                                                                     { false, ChildRoleCount::kMetadataEntry },
                                                                     { false, RouterRoleCount::kMetadataEntry },
                                                                     { false, LeaderRoleCount::kMetadataEntry },
                                                                     { false, AttachAttemptCount::kMetadataEntry },
                                                                     { false, PartitionIdChangeCount::kMetadataEntry },
                                                                     { false, BetterPartitionAttachAttemptCount::kMetadataEntry },
                                                                     { false, ParentChangeCount::kMetadataEntry },
                                                                     { false, TxTotalCount::kMetadataEntry },
                                                                     { false, TxUnicastCount::kMetadataEntry },
                                                                     { false, TxBroadcastCount::kMetadataEntry },
                                                                     { false, TxAckRequestedCount::kMetadataEntry },
                                                                     { false, TxAckedCount::kMetadataEntry },
                                                                     { false, TxNoAckRequestedCount::kMetadataEntry },
                                                                     { false, TxDataCount::kMetadataEntry },
                                                                     { false, TxDataPollCount::kMetadataEntry },
                                                                     { false, TxBeaconCount::kMetadataEntry },
                                                                     { false, TxBeaconRequestCount::kMetadataEntry },
                                                                     { false, TxOtherCount::kMetadataEntry },
                                                                     { false, TxRetryCount::kMetadataEntry },
                                                                     { false, TxDirectMaxRetryExpiryCount::kMetadataEntry },
                                                                     { false, TxIndirectMaxRetryExpiryCount::kMetadataEntry },
                                                                     { false, TxErrCcaCount::kMetadataEntry },
                                                                     { false, TxErrAbortCount::kMetadataEntry },
                                                                     { false, TxErrBusyChannelCount::kMetadataEntry },
                                                                     { false, RxTotalCount::kMetadataEntry },
                                                                     { false, RxUnicastCount::kMetadataEntry },
                                                                     { false, RxBroadcastCount::kMetadataEntry },
                                                                     { false, RxDataCount::kMetadataEntry },
                                                                     { false, RxDataPollCount::kMetadataEntry },
                                                                     { false, RxBeaconCount::kMetadataEntry },
                                                                     { false, RxBeaconRequestCount::kMetadataEntry },
                                                                     { false, RxOtherCount::kMetadataEntry },
                                                                     { false, RxAddressFilteredCount::kMetadataEntry },
                                                                     { false, RxDestAddrFilteredCount::kMetadataEntry },
                                                                     { false, RxDuplicatedCount::kMetadataEntry },
                                                                     { false, RxErrNoFrameCount::kMetadataEntry },
                                                                     { false, RxErrUnknownNeighborCount::kMetadataEntry },
                                                                     { false, RxErrInvalidSrcAddrCount::kMetadataEntry },
                                                                     { false, RxErrSecCount::kMetadataEntry },
                                                                     { false, RxErrFcsCount::kMetadataEntry },
                                                                     { false, RxErrOtherCount::kMetadataEntry },
                                                                     { false, OverrunCount::kMetadataEntry } };

} // namespace

CHIP_ERROR ThreadNetworkDiagnosticsCluster::Attributes(const ConcreteClusterPath & path,
                                                       ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder attributeListBuilder(builder);

    constexpr size_t NUM_ATTRIBUTES = sizeof(gAttributes) / sizeof(AttributeFeature);
    for (size_t i = 0; i < NUM_ATTRIBUTES; ++i)
    {
        if (gAttributes[i].id != OverrunCount::Id)
        {
            gAttributeEntries[i].enabled = mOptionalAttributes.test(gAttributes[i].id);
            if (gAttributes[i].feature.has_value())
            {
                gAttributeEntries[i].enabled = gAttributeEntries[i].enabled && mFeatures.Has(gAttributes[i].feature.value());
            }
        }
        else if (gAttributes[i].feature.has_value())
        {
            gAttributeEntries[i].enabled = mFeatures.Has(gAttributes[i].feature.value());
        }
    }

    return attributeListBuilder.Append(Span(kMandatoryMetadata), Span(gAttributeEntries));
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
