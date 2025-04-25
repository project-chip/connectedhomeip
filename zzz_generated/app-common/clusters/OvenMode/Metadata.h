// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OvenMode (cluster code: 73/0x49)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/OvenMode/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OvenMode {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace SupportedModes {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SupportedModes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedModes
namespace CurrentMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentMode

} // namespace Attributes

namespace Commands {
namespace ChangeToMode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ChangeToMode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ChangeToMode

} // namespace Commands
} // namespace OvenMode
} // namespace Clusters
} // namespace app
} // namespace chip
