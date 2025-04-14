// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThreadNetworkDirectory (cluster code: 1107/0x453)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ThreadNetworkDirectory/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadNetworkDirectory {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace PreferredExtendedPanID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::PreferredExtendedPanID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace PreferredExtendedPanID
namespace ThreadNetworks {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ThreadNetworks::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kOperate,
    .writePrivilege = std::nullopt,
};
} // namespace ThreadNetworks
namespace ThreadNetworkTableSize {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ThreadNetworkTableSize::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ThreadNetworkTableSize

} // namespace Attributes

namespace Commands {
namespace AddNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::AddNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace AddNetwork
namespace RemoveNetwork {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::RemoveNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace RemoveNetwork
namespace GetOperationalDataset {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::GetOperationalDataset::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GetOperationalDataset

} // namespace Commands
} // namespace ThreadNetworkDirectory
} // namespace Clusters
} // namespace app
} // namespace chip
