// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster RvcCleanMode (cluster code: 85/0x55)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/RvcCleanMode/RvcCleanModeIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace RvcCleanMode {
namespace Metadata {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kSupportedModesEntry = {
    .attributeId    = RvcCleanMode::Attributes::SupportedModes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentModeEntry = {
    .attributeId    = RvcCleanMode::Attributes::CurrentMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kChangeToModeEntry = {
    .commandId       = RvcCleanMode::Commands::ChangeToMode::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace RvcCleanMode
} // namespace clusters
} // namespace app
} // namespace chip
