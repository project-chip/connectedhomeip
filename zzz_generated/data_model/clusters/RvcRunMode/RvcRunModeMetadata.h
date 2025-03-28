// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster RvcRunMode (cluster code: 84/0x54)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/RvcRunMode/RvcRunModeIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace RvcRunMode {
namespace Metadata {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kSupportedModesEntry = {
    .attributeId    = RvcRunMode::Attributes::SupportedModes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentModeEntry = {
    .attributeId    = RvcRunMode::Attributes::CurrentMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kChangeToModeEntry = {
    .commandId       = RvcRunMode::Commands::ChangeToMode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace RvcRunMode
} // namespace clusters
} // namespace app
} // namespace chip
