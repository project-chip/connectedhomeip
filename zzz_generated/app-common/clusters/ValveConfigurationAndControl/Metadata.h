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

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace OpenDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OpenDuration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OpenDuration
namespace DefaultOpenDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DefaultOpenDuration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace DefaultOpenDuration
namespace AutoCloseTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AutoCloseTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AutoCloseTime
namespace RemainingDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RemainingDuration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RemainingDuration
namespace CurrentState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentState
namespace TargetState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TargetState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TargetState
namespace CurrentLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentLevel
namespace TargetLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TargetLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TargetLevel
namespace DefaultOpenLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DefaultOpenLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace DefaultOpenLevel
namespace ValveFault {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ValveFault::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ValveFault
namespace LevelStep {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LevelStep::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LevelStep

} // namespace Attributes

namespace Commands {
namespace Open {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Open::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Open
namespace Close {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Close::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Close

} // namespace Commands
} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
