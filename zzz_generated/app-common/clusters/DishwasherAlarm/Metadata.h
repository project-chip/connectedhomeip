// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DishwasherAlarm (cluster code: 93/0x5D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/DishwasherAlarm/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DishwasherAlarm {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace Mask {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Mask::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Mask
namespace Latch {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Latch::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Latch
namespace State {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = State::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace State
namespace Supported {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Supported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Supported

} // namespace Attributes

namespace Commands {
namespace Reset {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Reset::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Reset
namespace ModifyEnabledAlarms {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ModifyEnabledAlarms::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ModifyEnabledAlarms

} // namespace Commands
} // namespace DishwasherAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
