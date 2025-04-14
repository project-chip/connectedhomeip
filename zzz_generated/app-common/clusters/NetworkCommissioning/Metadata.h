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

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace MaxNetworks {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MaxNetworks::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace MaxNetworks
namespace Networks {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Networks::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace Networks
namespace ScanMaxTimeSeconds {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ScanMaxTimeSeconds::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ScanMaxTimeSeconds
namespace ConnectMaxTimeSeconds {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ConnectMaxTimeSeconds::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ConnectMaxTimeSeconds
namespace InterfaceEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::InterfaceEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
} // namespace InterfaceEnabled
namespace LastNetworkingStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::LastNetworkingStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace LastNetworkingStatus
namespace LastNetworkID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::LastNetworkID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace LastNetworkID
namespace LastConnectErrorValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::LastConnectErrorValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace LastConnectErrorValue
namespace SupportedWiFiBands {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SupportedWiFiBands::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedWiFiBands
namespace SupportedThreadFeatures {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SupportedThreadFeatures::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedThreadFeatures
namespace ThreadVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ThreadVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ThreadVersion

} // namespace Attributes

namespace Commands {
namespace ScanNetworks {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::ScanNetworks::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ScanNetworks
namespace AddOrUpdateWiFiNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::AddOrUpdateWiFiNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AddOrUpdateWiFiNetwork
namespace AddOrUpdateThreadNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::AddOrUpdateThreadNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AddOrUpdateThreadNetwork
namespace RemoveNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::RemoveNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace RemoveNetwork
namespace ConnectNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::ConnectNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ConnectNetwork
namespace ReorderNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::ReorderNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ReorderNetwork
namespace QueryIdentity {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::QueryIdentity::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace QueryIdentity

} // namespace Commands
} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
