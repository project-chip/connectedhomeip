/**
 *
 *    Copyright (c) 2021-2026 Project CHIP Authors
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
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/ThreadNetworkDiagnostics/AttributeIds.h>
#include <clusters/ThreadNetworkDiagnostics/Metadata.h>
#include <cstdint>
#include <lib/support/BitFlags.h>
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

namespace ThreadNetworkDiagnostics {

// This cluster has a LOT of attributes, however what we have is:
//   - all are sequential 0x0000 == Channel to 0x0040 == Rloc16
//   - All have RV permissions (No quality flags, privilege view, no write privilege), except lists which have a list privilege
//
// If we use kMetadataEntry we pay 8 bytes for metadata, but really we need 1 byte per attribute
// AND we only really need some ranges and "is list" checks.
//
// We do all this to save flash (about 600 bytes) and rely on unit tests to validate
// that the resulting attributes are actually correct.

struct AttributeIdRange
{
    uint8_t startInclusive;
    uint8_t endInclusive;
};

// we will rely on unit tests to validate that we actually cover everything correctly
constexpr AttributeIdRange kFullAttributeRange{
    .startInclusive = Attributes::Channel::Id, //
    .endInclusive   = Attributes::Rloc16::Id,  //
};

// mandatory attributes are in a few ranges, added here
constexpr AttributeIdRange kMandatoryAttributeRanges[] = {
    {
        .startInclusive = Attributes::Channel::Id,
        .endInclusive   = Attributes::MeshLocalPrefix::Id,
    },
    {
        .startInclusive = Attributes::NeighborTable::Id,
        .endInclusive   = Attributes::LeaderRouterId::Id,
    },
    {
        .startInclusive = Attributes::SecurityPolicy::Id,
        .endInclusive   = Attributes::Rloc16::Id,
    },
};

CHIP_ERROR AppendAllInRange(const AttributeIdRange range, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    for (uint8_t id = range.startInclusive; id <= range.endInclusive; id++)
    {
        bool isList = (                                     //
            (id == Attributes::NeighborTable::Id) ||        //
            (id == Attributes::RouteTable::Id) ||           //
            (id == Attributes::ActiveNetworkFaultsList::Id) //
        );

        ReturnErrorOnFailure(builder.Append(DataModel::AttributeEntry{
            id,
            isList ? BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute)
                   : BitFlags<DataModel::AttributeQualityFlags>(),
            Access::Privilege::kView, std::nullopt }));
    }
    return CHIP_NO_ERROR;
}

} // namespace ThreadNetworkDiagnostics

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
        return encoder.Encode(mClusterType == ClusterType::kMinimal ? BitFlags<Feature>() : kFeaturesAll);
    default:
        // Since ReadAttribute() is invoked only for valid attributes this is safe
        return WriteThreadNetworkDiagnosticAttributeToTlv(request.path.mAttributeId, encoder);
    }
}

CHIP_ERROR ThreadNetworkDiagnosticsCluster::Attributes(const ConcreteClusterPath & path,
                                                       ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    // pre-allocate for all
    ReturnErrorOnFailure(
        builder.EnsureAppendCapacity(kFullAttributeRange.endInclusive + DefaultServerCluster::GlobalAttributes().size()));

    // We only support minimal and full set of attributes because of flash considerations
    if (mClusterType == ClusterType::kMinimal)
    {
        for (const auto & range : kMandatoryAttributeRanges)
        {
            ReturnErrorOnFailure(AppendAllInRange(range, builder));
        }
    }
    else
    {
        ReturnErrorOnFailure(AppendAllInRange(kFullAttributeRange, builder));
    }

    // NOTE: ReferenceExisting will APPEND data (and use heap) when some data already
    //       exists in the builder.
    return builder.ReferenceExisting(DefaultServerCluster::GlobalAttributes());
}

// Notified when the Node’s connection status to a Thread network has changed.
void ThreadNetworkDiagnosticsCluster::OnConnectionStatusChanged(ConnectionStatusEnum newConnectionStatus)
{
    ChipLogProgress(Zcl, "ThdDiag: OnConnectionStatusChanged");

    VerifyOrReturn(mContext != nullptr);
    Events::ConnectionStatus::Type event{ newConnectionStatus };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

// Notified when the Node’s faults related to a Thread network have changed.
void ThreadNetworkDiagnosticsCluster::OnNetworkFaultChanged(const GeneralFaults<kMaxNetworkFaults> & previous,
                                                            const GeneralFaults<kMaxNetworkFaults> & current)
{
    ChipLogProgress(Zcl, "ThdDiag: OnNetworkFaultChanged");

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
