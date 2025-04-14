// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OccupancySensing (cluster code: 1030/0x406)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/OccupancySensing/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OccupancySensing {

inline constexpr uint32_t kRevision = 5;

namespace Attributes {
namespace Occupancy {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Occupancy::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Occupancy
namespace OccupancySensorType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::OccupancySensorType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OccupancySensorType
namespace OccupancySensorTypeBitmap {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::OccupancySensorTypeBitmap::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OccupancySensorTypeBitmap
namespace HoldTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::HoldTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace HoldTime
namespace HoldTimeLimits {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::HoldTimeLimits::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace HoldTimeLimits
namespace PIROccupiedToUnoccupiedDelay {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::PIROccupiedToUnoccupiedDelay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace PIROccupiedToUnoccupiedDelay
namespace PIRUnoccupiedToOccupiedDelay {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::PIRUnoccupiedToOccupiedDelay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace PIRUnoccupiedToOccupiedDelay
namespace PIRUnoccupiedToOccupiedThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::PIRUnoccupiedToOccupiedThreshold::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace PIRUnoccupiedToOccupiedThreshold
namespace UltrasonicOccupiedToUnoccupiedDelay {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::UltrasonicOccupiedToUnoccupiedDelay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace UltrasonicOccupiedToUnoccupiedDelay
namespace UltrasonicUnoccupiedToOccupiedDelay {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::UltrasonicUnoccupiedToOccupiedDelay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace UltrasonicUnoccupiedToOccupiedDelay
namespace UltrasonicUnoccupiedToOccupiedThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::UltrasonicUnoccupiedToOccupiedThreshold::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace UltrasonicUnoccupiedToOccupiedThreshold
namespace PhysicalContactOccupiedToUnoccupiedDelay {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::PhysicalContactOccupiedToUnoccupiedDelay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace PhysicalContactOccupiedToUnoccupiedDelay
namespace PhysicalContactUnoccupiedToOccupiedDelay {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::PhysicalContactUnoccupiedToOccupiedDelay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace PhysicalContactUnoccupiedToOccupiedDelay
namespace PhysicalContactUnoccupiedToOccupiedThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::PhysicalContactUnoccupiedToOccupiedThreshold::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace PhysicalContactUnoccupiedToOccupiedThreshold

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace OccupancySensing
} // namespace Clusters
} // namespace app
} // namespace chip
