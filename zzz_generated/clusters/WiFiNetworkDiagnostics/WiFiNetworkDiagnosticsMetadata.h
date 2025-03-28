// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WiFiNetworkDiagnostics (cluster code: 54/0x36)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WiFiNetworkDiagnostics/WiFiNetworkDiagnosticsIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace WiFiNetworkDiagnostics {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kBssidEntry = {
    .attributeId    = WiFiNetworkDiagnostics::Attributes::Bssid::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSecurityTypeEntry = {
    .attributeId    = WiFiNetworkDiagnostics::Attributes::SecurityType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kWiFiVersionEntry = {
    .attributeId    = WiFiNetworkDiagnostics::Attributes::WiFiVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kChannelNumberEntry = {
    .attributeId    = WiFiNetworkDiagnostics::Attributes::ChannelNumber::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRssiEntry = {
    .attributeId    = WiFiNetworkDiagnostics::Attributes::Rssi::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kBeaconLostCountEntry = {
    .attributeId    = WiFiNetworkDiagnostics::Attributes::BeaconLostCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kBeaconRxCountEntry = {
    .attributeId    = WiFiNetworkDiagnostics::Attributes::BeaconRxCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPacketMulticastRxCountEntry = {
    .attributeId    = WiFiNetworkDiagnostics::Attributes::PacketMulticastRxCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPacketMulticastTxCountEntry = {
    .attributeId    = WiFiNetworkDiagnostics::Attributes::PacketMulticastTxCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPacketUnicastRxCountEntry = {
    .attributeId    = WiFiNetworkDiagnostics::Attributes::PacketUnicastRxCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPacketUnicastTxCountEntry = {
    .attributeId    = WiFiNetworkDiagnostics::Attributes::PacketUnicastTxCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentMaxRateEntry = {
    .attributeId    = WiFiNetworkDiagnostics::Attributes::CurrentMaxRate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOverrunCountEntry = {
    .attributeId    = WiFiNetworkDiagnostics::Attributes::OverrunCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kResetCountsEntry = {
    .commandId       = WiFiNetworkDiagnostics::Commands::ResetCounts::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace WiFiNetworkDiagnostics
} // namespace clusters
} // namespace app
} // namespace chip
