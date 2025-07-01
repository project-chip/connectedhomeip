// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ValveConfigurationAndControl (cluster code: 129/0x81)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ValveConfigurationAndControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ValveConfigurationAndControl {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace OpenDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OpenDuration::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OpenDuration
namespace DefaultOpenDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DefaultOpenDuration::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace DefaultOpenDuration
namespace AutoCloseTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AutoCloseTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AutoCloseTime
namespace RemainingDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RemainingDuration::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RemainingDuration
namespace CurrentState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentState
namespace TargetState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TargetState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TargetState
namespace CurrentLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentLevel
namespace TargetLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TargetLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TargetLevel
namespace DefaultOpenLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DefaultOpenLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace DefaultOpenLevel
namespace ValveFault {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ValveFault::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ValveFault
namespace LevelStep {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LevelStep::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace LevelStep

} // namespace Attributes

namespace Commands {
namespace Open {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Open::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Open
namespace Close {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Close::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Close

} // namespace Commands
} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
