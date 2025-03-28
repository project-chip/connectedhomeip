// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThreadNetworkDirectory (cluster code: 1107/0x453)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ThreadNetworkDirectory/ThreadNetworkDirectoryIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace ThreadNetworkDirectory {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kPreferredExtendedPanIDEntry = {
    .attributeId    = ThreadNetworkDirectory::Attributes::PreferredExtendedPanID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kThreadNetworksEntry = {
    .attributeId    = ThreadNetworkDirectory::Attributes::ThreadNetworks::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kOperate,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kThreadNetworkTableSizeEntry = {
    .attributeId    = ThreadNetworkDirectory::Attributes::ThreadNetworkTableSize::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kAddNetworkEntry = {
    .commandId       = ThreadNetworkDirectory::Commands::AddNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kRemoveNetworkEntry = {
    .commandId       = ThreadNetworkDirectory::Commands::RemoveNetwork::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kGetOperationalDatasetEntry = {
    .commandId       = ThreadNetworkDirectory::Commands::GetOperationalDataset::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace ThreadNetworkDirectory
} // namespace clusters
} // namespace app
} // namespace chip
