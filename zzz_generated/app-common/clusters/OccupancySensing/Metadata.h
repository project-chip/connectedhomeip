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
inline constexpr DataModel::AttributeEntry kMetadataEntry(Occupancy::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Occupancy
namespace OccupancySensorType {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OccupancySensorType::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OccupancySensorType
namespace OccupancySensorTypeBitmap {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OccupancySensorTypeBitmap::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace OccupancySensorTypeBitmap
namespace HoldTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(HoldTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace HoldTime
namespace HoldTimeLimits {
inline constexpr DataModel::AttributeEntry kMetadataEntry(HoldTimeLimits::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace HoldTimeLimits
namespace PIROccupiedToUnoccupiedDelay {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PIROccupiedToUnoccupiedDelay::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace PIROccupiedToUnoccupiedDelay
namespace PIRUnoccupiedToOccupiedDelay {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PIRUnoccupiedToOccupiedDelay::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace PIRUnoccupiedToOccupiedDelay
namespace PIRUnoccupiedToOccupiedThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PIRUnoccupiedToOccupiedThreshold::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace PIRUnoccupiedToOccupiedThreshold
namespace UltrasonicOccupiedToUnoccupiedDelay {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UltrasonicOccupiedToUnoccupiedDelay::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace UltrasonicOccupiedToUnoccupiedDelay
namespace UltrasonicUnoccupiedToOccupiedDelay {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UltrasonicUnoccupiedToOccupiedDelay::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace UltrasonicUnoccupiedToOccupiedDelay
namespace UltrasonicUnoccupiedToOccupiedThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UltrasonicUnoccupiedToOccupiedThreshold::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace UltrasonicUnoccupiedToOccupiedThreshold
namespace PhysicalContactOccupiedToUnoccupiedDelay {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PhysicalContactOccupiedToUnoccupiedDelay::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace PhysicalContactOccupiedToUnoccupiedDelay
namespace PhysicalContactUnoccupiedToOccupiedDelay {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PhysicalContactUnoccupiedToOccupiedDelay::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace PhysicalContactUnoccupiedToOccupiedDelay
namespace PhysicalContactUnoccupiedToOccupiedThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PhysicalContactUnoccupiedToOccupiedThreshold::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace PhysicalContactUnoccupiedToOccupiedThreshold

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace OccupancySensing
} // namespace Clusters
} // namespace app
} // namespace chip
