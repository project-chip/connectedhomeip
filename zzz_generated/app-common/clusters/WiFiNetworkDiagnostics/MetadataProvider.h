// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WiFiNetworkDiagnostics (cluster code: 54/0x36)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/WiFiNetworkDiagnostics/Ids.h>
#include <clusters/WiFiNetworkDiagnostics/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::WiFiNetworkDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::WiFiNetworkDiagnostics::Attributes;
        switch (attributeId)
        {
        case Bssid::Id:
            return Bssid::kMetadataEntry;
        case SecurityType::Id:
            return SecurityType::kMetadataEntry;
        case WiFiVersion::Id:
            return WiFiVersion::kMetadataEntry;
        case ChannelNumber::Id:
            return ChannelNumber::kMetadataEntry;
        case Rssi::Id:
            return Rssi::kMetadataEntry;
        case BeaconLostCount::Id:
            return BeaconLostCount::kMetadataEntry;
        case BeaconRxCount::Id:
            return BeaconRxCount::kMetadataEntry;
        case PacketMulticastRxCount::Id:
            return PacketMulticastRxCount::kMetadataEntry;
        case PacketMulticastTxCount::Id:
            return PacketMulticastTxCount::kMetadataEntry;
        case PacketUnicastRxCount::Id:
            return PacketUnicastRxCount::kMetadataEntry;
        case PacketUnicastTxCount::Id:
            return PacketUnicastTxCount::kMetadataEntry;
        case CurrentMaxRate::Id:
            return CurrentMaxRate::kMetadataEntry;
        case OverrunCount::Id:
            return OverrunCount::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::WiFiNetworkDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::WiFiNetworkDiagnostics::Commands;
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
