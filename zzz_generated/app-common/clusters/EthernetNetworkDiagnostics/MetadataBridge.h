// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster EthernetNetworkDiagnostics (cluster code: 55/0x37)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/EthernetNetworkDiagnostics/Ids.h>
#include <clusters/EthernetNetworkDiagnostics/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::EthernetNetworkDiagnostics::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::EthernetNetworkDiagnostics::Attributes;
        switch (commandId)
        {
        case PHYRate::Id:
            return PHYRate::kMetadataEntry;
        case FullDuplex::Id:
            return FullDuplex::kMetadataEntry;
        case PacketRxCount::Id:
            return PacketRxCount::kMetadataEntry;
        case PacketTxCount::Id:
            return PacketTxCount::kMetadataEntry;
        case TxErrCount::Id:
            return TxErrCount::kMetadataEntry;
        case CollisionCount::Id:
            return CollisionCount::kMetadataEntry;
        case OverrunCount::Id:
            return OverrunCount::kMetadataEntry;
        case CarrierDetect::Id:
            return CarrierDetect::kMetadataEntry;
        case TimeSinceReset::Id:
            return TimeSinceReset::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::EthernetNetworkDiagnostics::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::EthernetNetworkDiagnostics::Commands;
        switch (commandId)
        {
        case ResetCounts::Id:
            return ResetCounts::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
