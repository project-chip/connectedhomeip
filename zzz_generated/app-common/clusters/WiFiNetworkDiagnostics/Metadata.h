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
inline constexpr DataModel::AttributeEntry kMetadataEntry(Bssid::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Bssid
namespace SecurityType {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SecurityType::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SecurityType
namespace WiFiVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WiFiVersion::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace WiFiVersion
namespace ChannelNumber {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ChannelNumber::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ChannelNumber
namespace Rssi {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Rssi::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Rssi
namespace BeaconLostCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BeaconLostCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BeaconLostCount
namespace BeaconRxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BeaconRxCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BeaconRxCount
namespace PacketMulticastRxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PacketMulticastRxCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PacketMulticastRxCount
namespace PacketMulticastTxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PacketMulticastTxCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PacketMulticastTxCount
namespace PacketUnicastRxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PacketUnicastRxCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PacketUnicastRxCount
namespace PacketUnicastTxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PacketUnicastTxCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PacketUnicastTxCount
namespace CurrentMaxRate {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentMaxRate::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentMaxRate
namespace OverrunCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OverrunCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OverrunCount

} // namespace Attributes

namespace Commands {
namespace ResetCounts {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ResetCounts::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ResetCounts

} // namespace Commands
} // namespace WiFiNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
