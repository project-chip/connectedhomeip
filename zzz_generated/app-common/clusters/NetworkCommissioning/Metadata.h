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
    .attributeId    = MaxNetworks::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace MaxNetworks
namespace Networks {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Networks::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace Networks
namespace ScanMaxTimeSeconds {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ScanMaxTimeSeconds::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ScanMaxTimeSeconds
namespace ConnectMaxTimeSeconds {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ConnectMaxTimeSeconds::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ConnectMaxTimeSeconds
namespace InterfaceEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = InterfaceEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
} // namespace InterfaceEnabled
namespace LastNetworkingStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LastNetworkingStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace LastNetworkingStatus
namespace LastNetworkID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LastNetworkID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace LastNetworkID
namespace LastConnectErrorValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LastConnectErrorValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace LastConnectErrorValue
namespace SupportedWiFiBands {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SupportedWiFiBands::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedWiFiBands
namespace SupportedThreadFeatures {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SupportedThreadFeatures::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedThreadFeatures
namespace ThreadVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThreadVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ThreadVersion

} // namespace Attributes

namespace Commands {
namespace ScanNetworks {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ScanNetworks::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ScanNetworks
namespace AddOrUpdateWiFiNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AddOrUpdateWiFiNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AddOrUpdateWiFiNetwork
namespace AddOrUpdateThreadNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AddOrUpdateThreadNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AddOrUpdateThreadNetwork
namespace RemoveNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RemoveNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace RemoveNetwork
namespace ConnectNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ConnectNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ConnectNetwork
namespace ReorderNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ReorderNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ReorderNetwork
namespace QueryIdentity {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = QueryIdentity::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace QueryIdentity

} // namespace Commands
} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
