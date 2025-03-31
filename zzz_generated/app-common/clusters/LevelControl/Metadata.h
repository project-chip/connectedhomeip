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
namespace clusters {
namespace LevelControl {

inline constexpr uint32_t kRevision = 6;

namespace Attributes {
namespace CurrentLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelControl::Attributes::CurrentLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentLevel
namespace RemainingTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelControl::Attributes::RemainingTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RemainingTime
namespace MinLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelControl::Attributes::MinLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinLevel
namespace MaxLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelControl::Attributes::MaxLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxLevel
namespace CurrentFrequency {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelControl::Attributes::CurrentFrequency::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentFrequency
namespace MinFrequency {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelControl::Attributes::MinFrequency::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinFrequency
namespace MaxFrequency {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelControl::Attributes::MaxFrequency::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxFrequency
namespace Options {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelControl::Attributes::Options::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace Options
namespace OnOffTransitionTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelControl::Attributes::OnOffTransitionTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace OnOffTransitionTime
namespace OnLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelControl::Attributes::OnLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace OnLevel
namespace OnTransitionTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelControl::Attributes::OnTransitionTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace OnTransitionTime
namespace OffTransitionTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelControl::Attributes::OffTransitionTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace OffTransitionTime
namespace DefaultMoveRate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelControl::Attributes::DefaultMoveRate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace DefaultMoveRate
namespace StartUpCurrentLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelControl::Attributes::StartUpCurrentLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace StartUpCurrentLevel

} // namespace Attributes

namespace Commands {
namespace MoveToLevel {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = LevelControl::Commands::MoveToLevel::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MoveToLevel
namespace Move {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = LevelControl::Commands::Move::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Move
namespace Step {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = LevelControl::Commands::Step::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Step
namespace Stop {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = LevelControl::Commands::Stop::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Stop
namespace MoveToLevelWithOnOff {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = LevelControl::Commands::MoveToLevelWithOnOff::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MoveToLevelWithOnOff
namespace MoveWithOnOff {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = LevelControl::Commands::MoveWithOnOff::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MoveWithOnOff
namespace StepWithOnOff {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = LevelControl::Commands::StepWithOnOff::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StepWithOnOff
namespace StopWithOnOff {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = LevelControl::Commands::StopWithOnOff::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StopWithOnOff
namespace MoveToClosestFrequency {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = LevelControl::Commands::MoveToClosestFrequency::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MoveToClosestFrequency

} // namespace Commands
} // namespace LevelControl
} // namespace clusters
} // namespace app
} // namespace chip
