// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Actions (cluster code: 37/0x25)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Actions/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Actions {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace ActionList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActionList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActionList
namespace EndpointLists {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EndpointLists::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace EndpointLists
namespace SetupURL {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SetupURL::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SetupURL

} // namespace Attributes

namespace Commands {
namespace InstantAction {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = InstantAction::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace InstantAction
namespace InstantActionWithTransition {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = InstantActionWithTransition::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace InstantActionWithTransition
namespace StartAction {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = StartAction::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StartAction
namespace StartActionWithDuration {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = StartActionWithDuration::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StartActionWithDuration
namespace StopAction {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = StopAction::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StopAction
namespace PauseAction {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = PauseAction::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace PauseAction
namespace PauseActionWithDuration {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = PauseActionWithDuration::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace PauseActionWithDuration
namespace ResumeAction {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ResumeAction::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ResumeAction
namespace EnableAction {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = EnableAction::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace EnableAction
namespace EnableActionWithDuration {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = EnableActionWithDuration::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace EnableActionWithDuration
namespace DisableAction {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = DisableAction::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace DisableAction
namespace DisableActionWithDuration {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = DisableActionWithDuration::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace DisableActionWithDuration

} // namespace Commands
} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip
