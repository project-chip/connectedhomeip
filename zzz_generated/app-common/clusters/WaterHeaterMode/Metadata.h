// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WaterHeaterMode (cluster code: 158/0x9E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/WaterHeaterMode/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterMode {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace SupportedModes {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SupportedModes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedModes
namespace CurrentMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CurrentMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentMode

} // namespace Attributes

namespace Commands {
namespace ChangeToMode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::ChangeToMode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ChangeToMode

} // namespace Commands
} // namespace WaterHeaterMode
} // namespace Clusters
} // namespace app
} // namespace chip
