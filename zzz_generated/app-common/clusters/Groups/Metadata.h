// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Groups (cluster code: 4/0x4)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Groups/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Groups {

inline constexpr uint32_t kRevision = 4;

namespace Attributes {
namespace NameSupport {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NameSupport::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NameSupport

} // namespace Attributes

namespace Commands {
namespace AddGroup {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(AddGroup::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace AddGroup
namespace ViewGroup {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ViewGroup::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace ViewGroup
namespace GetGroupMembership {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(GetGroupMembership::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace GetGroupMembership
namespace RemoveGroup {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RemoveGroup::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace RemoveGroup
namespace RemoveAllGroups {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RemoveAllGroups::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace RemoveAllGroups
namespace AddGroupIfIdentifying {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(AddGroupIfIdentifying::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace AddGroupIfIdentifying

} // namespace Commands
} // namespace Groups
} // namespace Clusters
} // namespace app
} // namespace chip
