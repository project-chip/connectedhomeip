// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OccupancySensing (cluster code: 1030/0x406)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/OccupancySensing/OccupancySensingIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace OccupancySensing {
namespace Metadata {

inline constexpr uint32_t kRevision = 5;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kOccupancyEntry = {
    .attributeId    = Attributes::Occupancy::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOccupancySensorTypeEntry = {
    .attributeId    = Attributes::OccupancySensorType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOccupancySensorTypeBitmapEntry = {
    .attributeId    = Attributes::OccupancySensorTypeBitmap::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kHoldTimeEntry = {
    .attributeId    = Attributes::HoldTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kHoldTimeLimitsEntry = {
    .attributeId    = Attributes::HoldTimeLimits::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPIROccupiedToUnoccupiedDelayEntry = {
    .attributeId    = Attributes::PIROccupiedToUnoccupiedDelay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kPIRUnoccupiedToOccupiedDelayEntry = {
    .attributeId    = Attributes::PIRUnoccupiedToOccupiedDelay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kPIRUnoccupiedToOccupiedThresholdEntry = {
    .attributeId    = Attributes::PIRUnoccupiedToOccupiedThreshold::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kUltrasonicOccupiedToUnoccupiedDelayEntry = {
    .attributeId    = Attributes::UltrasonicOccupiedToUnoccupiedDelay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kUltrasonicUnoccupiedToOccupiedDelayEntry = {
    .attributeId    = Attributes::UltrasonicUnoccupiedToOccupiedDelay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kUltrasonicUnoccupiedToOccupiedThresholdEntry = {
    .attributeId    = Attributes::UltrasonicUnoccupiedToOccupiedThreshold::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kPhysicalContactOccupiedToUnoccupiedDelayEntry = {
    .attributeId    = Attributes::PhysicalContactOccupiedToUnoccupiedDelay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kPhysicalContactUnoccupiedToOccupiedDelayEntry = {
    .attributeId    = Attributes::PhysicalContactUnoccupiedToOccupiedDelay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kPhysicalContactUnoccupiedToOccupiedThresholdEntry = {
    .attributeId    = Attributes::PhysicalContactUnoccupiedToOccupiedThreshold::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace OccupancySensing
} // namespace clusters
} // namespace app
} // namespace chip
