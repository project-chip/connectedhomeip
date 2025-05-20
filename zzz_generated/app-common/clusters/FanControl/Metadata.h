// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster FanControl (cluster code: 514/0x202)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/FanControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace FanControl {

inline constexpr uint32_t kRevision = 5;

namespace Attributes {
namespace FanMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(FanMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace FanMode
namespace FanModeSequence {
inline constexpr DataModel::AttributeEntry kMetadataEntry(FanModeSequence::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace FanModeSequence
namespace PercentSetting {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PercentSetting::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace PercentSetting
namespace PercentCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PercentCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PercentCurrent
namespace SpeedMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SpeedMax::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SpeedMax
namespace SpeedSetting {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SpeedSetting::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace SpeedSetting
namespace SpeedCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SpeedCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SpeedCurrent
namespace RockSupport {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RockSupport::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RockSupport
namespace RockSetting {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RockSetting::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace RockSetting
namespace WindSupport {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WindSupport::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace WindSupport
namespace WindSetting {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WindSetting::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace WindSetting
namespace AirflowDirection {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AirflowDirection::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace AirflowDirection

} // namespace Attributes

namespace Commands {
namespace Step {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Step::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Step

} // namespace Commands
} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace chip
