// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WiFiNetworkDiagnostics (cluster code: 54/0x36)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WiFiNetworkDiagnostics/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WiFiNetworkDiagnostics {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace Bssid {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Bssid::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Bssid
namespace SecurityType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SecurityType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SecurityType
namespace WiFiVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = WiFiVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiFiVersion
namespace ChannelNumber {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ChannelNumber::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ChannelNumber
namespace Rssi {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Rssi::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Rssi
namespace BeaconLostCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BeaconLostCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BeaconLostCount
namespace BeaconRxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BeaconRxCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BeaconRxCount
namespace PacketMulticastRxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PacketMulticastRxCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PacketMulticastRxCount
namespace PacketMulticastTxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PacketMulticastTxCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PacketMulticastTxCount
namespace PacketUnicastRxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PacketUnicastRxCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PacketUnicastRxCount
namespace PacketUnicastTxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PacketUnicastTxCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PacketUnicastTxCount
namespace CurrentMaxRate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentMaxRate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentMaxRate
namespace OverrunCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OverrunCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OverrunCount

} // namespace Attributes

namespace Commands {
namespace ResetCounts {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = WiFiNetworkDiagnostics::Commands::ResetCounts::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ResetCounts

} // namespace Commands
} // namespace WiFiNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
