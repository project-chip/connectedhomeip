// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DishwasherAlarm (cluster code: 93/0x5D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/DishwasherAlarm/DishwasherAlarmIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace DishwasherAlarm {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kMaskEntry = {
    .attributeId    = DishwasherAlarm::Attributes::Mask::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLatchEntry = {
    .attributeId    = DishwasherAlarm::Attributes::Latch::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kStateEntry = {
    .attributeId    = DishwasherAlarm::Attributes::State::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSupportedEntry = {
    .attributeId    = DishwasherAlarm::Attributes::Supported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kResetEntry = {
    .commandId       = DishwasherAlarm::Commands::Reset::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kModifyEnabledAlarmsEntry = {
    .commandId       = DishwasherAlarm::Commands::ModifyEnabledAlarms::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace DishwasherAlarm
} // namespace clusters
} // namespace app
} // namespace chip
