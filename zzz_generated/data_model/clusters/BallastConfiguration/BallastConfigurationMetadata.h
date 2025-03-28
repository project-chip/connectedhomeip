// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster BallastConfiguration (cluster code: 769/0x301)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/BallastConfiguration/BallastConfigurationIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace BallastConfiguration {
namespace Metadata {

inline constexpr uint32_t kRevision = 4;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kPhysicalMinLevelEntry = {
    .attributeId    = BallastConfiguration::Attributes::PhysicalMinLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPhysicalMaxLevelEntry = {
    .attributeId    = BallastConfiguration::Attributes::PhysicalMaxLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kBallastStatusEntry = {
    .attributeId    = BallastConfiguration::Attributes::BallastStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMinLevelEntry = {
    .attributeId    = BallastConfiguration::Attributes::MinLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kMaxLevelEntry = {
    .attributeId    = BallastConfiguration::Attributes::MaxLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kIntrinsicBallastFactorEntry = {
    .attributeId    = BallastConfiguration::Attributes::IntrinsicBallastFactor::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kBallastFactorAdjustmentEntry = {
    .attributeId    = BallastConfiguration::Attributes::BallastFactorAdjustment::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kLampQuantityEntry = {
    .attributeId    = BallastConfiguration::Attributes::LampQuantity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLampTypeEntry = {
    .attributeId    = BallastConfiguration::Attributes::LampType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kLampManufacturerEntry = {
    .attributeId    = BallastConfiguration::Attributes::LampManufacturer::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kLampRatedHoursEntry = {
    .attributeId    = BallastConfiguration::Attributes::LampRatedHours::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kLampBurnHoursEntry = {
    .attributeId    = BallastConfiguration::Attributes::LampBurnHours::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kLampAlarmModeEntry = {
    .attributeId    = BallastConfiguration::Attributes::LampAlarmMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kLampBurnHoursTripPointEntry = {
    .attributeId    = BallastConfiguration::Attributes::LampBurnHoursTripPoint::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace BallastConfiguration
} // namespace clusters
} // namespace app
} // namespace chip
