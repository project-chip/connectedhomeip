// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster MediaInput (cluster code: 1287/0x507)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/MediaInput/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaInput {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace InputList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = InputList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace InputList
namespace CurrentInput {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentInput::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentInput

} // namespace Attributes

namespace Commands {
namespace SelectInput {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SelectInput::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SelectInput
namespace ShowInputStatus {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ShowInputStatus::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ShowInputStatus
namespace HideInputStatus {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = HideInputStatus::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace HideInputStatus
namespace RenameInput {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RenameInput::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace RenameInput

} // namespace Commands
} // namespace MediaInput
} // namespace Clusters
} // namespace app
} // namespace chip
