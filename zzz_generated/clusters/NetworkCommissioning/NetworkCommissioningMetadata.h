// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster NetworkCommissioning (cluster code: 49/0x31)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/NetworkCommissioning/NetworkCommissioningIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace NetworkCommissioning {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kMaxNetworksEntry = {
    .attributeId    = NetworkCommissioning::Attributes::MaxNetworks::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNetworksEntry = {
    .attributeId    = NetworkCommissioning::Attributes::Networks::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kScanMaxTimeSecondsEntry = {
    .attributeId    = NetworkCommissioning::Attributes::ScanMaxTimeSeconds::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kConnectMaxTimeSecondsEntry = {
    .attributeId    = NetworkCommissioning::Attributes::ConnectMaxTimeSeconds::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kInterfaceEnabledEntry = {
    .attributeId    = NetworkCommissioning::Attributes::InterfaceEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AttributeEntry kLastNetworkingStatusEntry = {
    .attributeId    = NetworkCommissioning::Attributes::LastNetworkingStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLastNetworkIDEntry = {
    .attributeId    = NetworkCommissioning::Attributes::LastNetworkID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLastConnectErrorValueEntry = {
    .attributeId    = NetworkCommissioning::Attributes::LastConnectErrorValue::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSupportedWiFiBandsEntry = {
    .attributeId    = NetworkCommissioning::Attributes::SupportedWiFiBands::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSupportedThreadFeaturesEntry = {
    .attributeId    = NetworkCommissioning::Attributes::SupportedThreadFeatures::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kThreadVersionEntry = {
    .attributeId    = NetworkCommissioning::Attributes::ThreadVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kScanNetworksEntry = {
    .commandId       = NetworkCommissioning::Commands::ScanNetworks::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kAddOrUpdateWiFiNetworkEntry = {
    .commandId       = NetworkCommissioning::Commands::AddOrUpdateWiFiNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kAddOrUpdateThreadNetworkEntry = {
    .commandId       = NetworkCommissioning::Commands::AddOrUpdateThreadNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kRemoveNetworkEntry = {
    .commandId       = NetworkCommissioning::Commands::RemoveNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kConnectNetworkEntry = {
    .commandId       = NetworkCommissioning::Commands::ConnectNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kReorderNetworkEntry = {
    .commandId       = NetworkCommissioning::Commands::ReorderNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kQueryIdentityEntry = {
    .commandId       = NetworkCommissioning::Commands::QueryIdentity::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};

} // namespace Commands
} // namespace Metadata
} // namespace NetworkCommissioning
} // namespace clusters
} // namespace app
} // namespace chip
