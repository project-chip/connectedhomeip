// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster BallastConfiguration (cluster code: 769/0x301)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/BallastConfiguration/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace BallastConfiguration {

inline constexpr uint32_t kRevision = 4;

namespace Attributes {
namespace PhysicalMinLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PhysicalMinLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PhysicalMinLevel
namespace PhysicalMaxLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PhysicalMaxLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PhysicalMaxLevel
namespace BallastStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BallastStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BallastStatus
namespace MinLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace MinLevel
namespace MaxLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace MaxLevel
namespace IntrinsicBallastFactor {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = IntrinsicBallastFactor::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace IntrinsicBallastFactor
namespace BallastFactorAdjustment {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BallastFactorAdjustment::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace BallastFactorAdjustment
namespace LampQuantity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LampQuantity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LampQuantity
namespace LampType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LampType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LampType
namespace LampManufacturer {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LampManufacturer::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LampManufacturer
namespace LampRatedHours {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LampRatedHours::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LampRatedHours
namespace LampBurnHours {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LampBurnHours::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LampBurnHours
namespace LampAlarmMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LampAlarmMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LampAlarmMode
namespace LampBurnHoursTripPoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LampBurnHoursTripPoint::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LampBurnHoursTripPoint

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace BallastConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip
