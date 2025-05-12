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
inline constexpr DataModel::AttributeEntry kMetadataEntry(PhysicalMinLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PhysicalMinLevel
namespace PhysicalMaxLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PhysicalMaxLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PhysicalMaxLevel
namespace BallastStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BallastStatus::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BallastStatus
namespace MinLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace MinLevel
namespace MaxLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace MaxLevel
namespace IntrinsicBallastFactor {
inline constexpr DataModel::AttributeEntry kMetadataEntry(IntrinsicBallastFactor::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace IntrinsicBallastFactor
namespace BallastFactorAdjustment {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BallastFactorAdjustment::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace BallastFactorAdjustment
namespace LampQuantity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LampQuantity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace LampQuantity
namespace LampType {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LampType::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace LampType
namespace LampManufacturer {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LampManufacturer::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace LampManufacturer
namespace LampRatedHours {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LampRatedHours::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace LampRatedHours
namespace LampBurnHours {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LampBurnHours::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace LampBurnHours
namespace LampAlarmMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LampAlarmMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace LampAlarmMode
namespace LampBurnHoursTripPoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LampBurnHoursTripPoint::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace LampBurnHoursTripPoint

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace BallastConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip
