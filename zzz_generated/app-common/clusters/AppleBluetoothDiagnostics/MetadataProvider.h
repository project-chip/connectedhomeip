// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AppleBluetoothDiagnostics (cluster code: 323615751/0x1349FC07)
// based on ../../../connectedhomeip/src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/AppleBluetoothDiagnostics/Ids.h>
#include <clusters/AppleBluetoothDiagnostics/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::AppleBluetoothDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::AppleBluetoothDiagnostics::Attributes;
        switch (attributeId)
        {
        case AppleBluetoothVersion::Id:
            return AppleBluetoothVersion::kMetadataEntry;
        case AppleBluetoothRSSI::Id:
            return AppleBluetoothRSSI::kMetadataEntry;
        case AppleBluetoothTxPowerLevel::Id:
            return AppleBluetoothTxPowerLevel::kMetadataEntry;
        case AppleBluetoothPacketRxCount::Id:
            return AppleBluetoothPacketRxCount::kMetadataEntry;
        case AppleBluetoothPacketTxCount::Id:
            return AppleBluetoothPacketTxCount::kMetadataEntry;
        case AppleBluetoothPacketRxErrorCount::Id:
            return AppleBluetoothPacketRxErrorCount::kMetadataEntry;
        case AppleBluetoothPacketTxErrorCount::Id:
            return AppleBluetoothPacketTxErrorCount::kMetadataEntry;
        case AppleBluetoothOverrunCount::Id:
            return AppleBluetoothOverrunCount::kMetadataEntry;
        case AppleBluetoothConnectionStatus::Id:
            return AppleBluetoothConnectionStatus::kMetadataEntry;
        case AppleBluetoothConnectionCount::Id:
            return AppleBluetoothConnectionCount::kMetadataEntry;
        case AppleBluetoothConnectionErrorCount::Id:
            return AppleBluetoothConnectionErrorCount::kMetadataEntry;
        case AppleBluetoothDisconnectionCount::Id:
            return AppleBluetoothDisconnectionCount::kMetadataEntry;
        case AppleBluetoothDisconnectionErrorCount::Id:
            return AppleBluetoothDisconnectionErrorCount::kMetadataEntry;
        case AppleBluetoothHardwareExceptionCount::Id:
            return AppleBluetoothHardwareExceptionCount::kMetadataEntry;
        case AppleBluetoothResetCountBootRelativeTime::Id:
            return AppleBluetoothResetCountBootRelativeTime::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::AppleBluetoothDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::AppleBluetoothDiagnostics::Commands;
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
