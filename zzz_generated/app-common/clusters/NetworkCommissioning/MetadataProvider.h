// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster NetworkCommissioning (cluster code: 49/0x31)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/NetworkCommissioning/Ids.h>
#include <clusters/NetworkCommissioning/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::NetworkCommissioning::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::NetworkCommissioning::Attributes;
        switch (attributeId)
        {
        case MaxNetworks::Id:
            return MaxNetworks::kMetadataEntry;
        case Networks::Id:
            return Networks::kMetadataEntry;
        case ScanMaxTimeSeconds::Id:
            return ScanMaxTimeSeconds::kMetadataEntry;
        case ConnectMaxTimeSeconds::Id:
            return ConnectMaxTimeSeconds::kMetadataEntry;
        case InterfaceEnabled::Id:
            return InterfaceEnabled::kMetadataEntry;
        case LastNetworkingStatus::Id:
            return LastNetworkingStatus::kMetadataEntry;
        case LastNetworkID::Id:
            return LastNetworkID::kMetadataEntry;
        case LastConnectErrorValue::Id:
            return LastConnectErrorValue::kMetadataEntry;
        case SupportedWiFiBands::Id:
            return SupportedWiFiBands::kMetadataEntry;
        case SupportedThreadFeatures::Id:
            return SupportedThreadFeatures::kMetadataEntry;
        case ThreadVersion::Id:
            return ThreadVersion::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::NetworkCommissioning::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::NetworkCommissioning::Commands;
        switch (commandId)
        {
        case ScanNetworks::Id:
            return ScanNetworks::kMetadataEntry;
        case AddOrUpdateWiFiNetwork::Id:
            return AddOrUpdateWiFiNetwork::kMetadataEntry;
        case AddOrUpdateThreadNetwork::Id:
            return AddOrUpdateThreadNetwork::kMetadataEntry;
        case RemoveNetwork::Id:
            return RemoveNetwork::kMetadataEntry;
        case ConnectNetwork::Id:
            return ConnectNetwork::kMetadataEntry;
        case ReorderNetwork::Id:
            return ReorderNetwork::kMetadataEntry;
        case QueryIdentity::Id:
            return QueryIdentity::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
