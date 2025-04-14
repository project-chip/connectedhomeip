// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AudioOutput (cluster code: 1291/0x50B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AudioOutput/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AudioOutput {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace OutputList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::OutputList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OutputList
namespace CurrentOutput {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CurrentOutput::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentOutput

} // namespace Attributes

namespace Commands {
namespace SelectOutput {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::SelectOutput::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SelectOutput
namespace RenameOutput {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::RenameOutput::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace RenameOutput

} // namespace Commands
} // namespace AudioOutput
} // namespace Clusters
} // namespace app
} // namespace chip
