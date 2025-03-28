// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster FanControl (cluster code: 514/0x202)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/FanControl/FanControlIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace FanControl {
namespace Metadata {

inline constexpr uint32_t kRevision = 5;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kFanModeEntry = {
    .attributeId    = FanControl::Attributes::FanMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kFanModeSequenceEntry = {
    .attributeId    = FanControl::Attributes::FanModeSequence::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPercentSettingEntry = {
    .attributeId    = FanControl::Attributes::PercentSetting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kPercentCurrentEntry = {
    .attributeId    = FanControl::Attributes::PercentCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSpeedMaxEntry = {
    .attributeId    = FanControl::Attributes::SpeedMax::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSpeedSettingEntry = {
    .attributeId    = FanControl::Attributes::SpeedSetting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kSpeedCurrentEntry = {
    .attributeId    = FanControl::Attributes::SpeedCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRockSupportEntry = {
    .attributeId    = FanControl::Attributes::RockSupport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRockSettingEntry = {
    .attributeId    = FanControl::Attributes::RockSetting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kWindSupportEntry = {
    .attributeId    = FanControl::Attributes::WindSupport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kWindSettingEntry = {
    .attributeId    = FanControl::Attributes::WindSetting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kAirflowDirectionEntry = {
    .attributeId    = FanControl::Attributes::AirflowDirection::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kStepEntry = {
    .commandId       = FanControl::Commands::Step::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace FanControl
} // namespace clusters
} // namespace app
} // namespace chip
