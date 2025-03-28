// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Identify (cluster code: 3/0x3)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Identify/IdentifyIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace Identify {
namespace Metadata {

inline constexpr uint32_t kRevision = 4;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kIdentifyTimeEntry = {
    .attributeId    = Identify::Attributes::IdentifyTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kIdentifyTypeEntry = {
    .attributeId    = Identify::Attributes::IdentifyType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kIdentifyEntry = {
    .commandId       = Identify::Commands::Identify::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kTriggerEffectEntry = {
    .commandId       = Identify::Commands::TriggerEffect::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands
} // namespace Metadata
} // namespace Identify
} // namespace clusters
} // namespace app
} // namespace chip
