// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster NetworkCommissioning (cluster code: 49/0x31)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/NetworkCommissioning/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace MaxNetworks {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxNetworks::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace MaxNetworks
namespace Networks {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Networks::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace Networks
namespace ScanMaxTimeSeconds {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ScanMaxTimeSeconds::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ScanMaxTimeSeconds
namespace ConnectMaxTimeSeconds {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ConnectMaxTimeSeconds::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ConnectMaxTimeSeconds
namespace InterfaceEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(InterfaceEnabled::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kAdminister);
} // namespace InterfaceEnabled
namespace LastNetworkingStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LastNetworkingStatus::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace LastNetworkingStatus
namespace LastNetworkID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LastNetworkID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace LastNetworkID
namespace LastConnectErrorValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LastConnectErrorValue::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, std::nullopt);
} // namespace LastConnectErrorValue
namespace SupportedWiFiBands {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SupportedWiFiBands::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SupportedWiFiBands
namespace SupportedThreadFeatures {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SupportedThreadFeatures::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SupportedThreadFeatures
namespace ThreadVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ThreadVersion::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ThreadVersion

} // namespace Attributes

namespace Commands {
namespace ScanNetworks {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ScanNetworks::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace ScanNetworks
namespace AddOrUpdateWiFiNetwork {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(AddOrUpdateWiFiNetwork::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace AddOrUpdateWiFiNetwork
namespace AddOrUpdateThreadNetwork {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(AddOrUpdateThreadNetwork::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace AddOrUpdateThreadNetwork
namespace RemoveNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RemoveNetwork::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace RemoveNetwork
namespace ConnectNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ConnectNetwork::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace ConnectNetwork
namespace ReorderNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ReorderNetwork::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace ReorderNetwork
namespace QueryIdentity {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(QueryIdentity::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace QueryIdentity

} // namespace Commands
} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
