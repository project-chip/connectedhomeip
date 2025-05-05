// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LevelControl (cluster code: 8/0x8)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/LevelControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LevelControl {

inline constexpr uint32_t kRevision = 6;

namespace Attributes {
namespace CurrentLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentLevel
namespace RemainingTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RemainingTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RemainingTime
namespace MinLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinLevel
namespace MaxLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxLevel
namespace CurrentFrequency {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentFrequency::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentFrequency
namespace MinFrequency {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinFrequency::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinFrequency
namespace MaxFrequency {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxFrequency::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxFrequency
namespace Options {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Options::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Options
namespace OnOffTransitionTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OnOffTransitionTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace OnOffTransitionTime
namespace OnLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OnLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace OnLevel
namespace OnTransitionTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OnTransitionTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace OnTransitionTime
namespace OffTransitionTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OffTransitionTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace OffTransitionTime
namespace DefaultMoveRate {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DefaultMoveRate::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace DefaultMoveRate
namespace StartUpCurrentLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StartUpCurrentLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace StartUpCurrentLevel

} // namespace Attributes

namespace Commands {
namespace MoveToLevel {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(MoveToLevel::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace MoveToLevel
namespace Move {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Move::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Move
namespace Step {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Step::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Step
namespace Stop {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Stop::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Stop
namespace MoveToLevelWithOnOff {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(MoveToLevelWithOnOff::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace MoveToLevelWithOnOff
namespace MoveWithOnOff {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(MoveWithOnOff::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace MoveWithOnOff
namespace StepWithOnOff {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StepWithOnOff::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StepWithOnOff
namespace StopWithOnOff {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StopWithOnOff::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StopWithOnOff
namespace MoveToClosestFrequency {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(MoveToClosestFrequency::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace MoveToClosestFrequency

} // namespace Commands
} // namespace LevelControl
} // namespace Clusters
} // namespace app
} // namespace chip
