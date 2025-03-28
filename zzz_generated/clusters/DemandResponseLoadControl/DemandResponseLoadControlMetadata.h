// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DemandResponseLoadControl (cluster code: 150/0x96)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/DemandResponseLoadControl/DemandResponseLoadControlIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace DemandResponseLoadControl {
namespace Metadata {

inline constexpr uint32_t kRevision = 4;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kLoadControlProgramsEntry = {
    .attributeId    = DemandResponseLoadControl::Attributes::LoadControlPrograms::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfLoadControlProgramsEntry = {
    .attributeId    = DemandResponseLoadControl::Attributes::NumberOfLoadControlPrograms::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kEventsEntry = {
    .attributeId    = DemandResponseLoadControl::Attributes::Events::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActiveEventsEntry = {
    .attributeId    = DemandResponseLoadControl::Attributes::ActiveEvents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfEventsPerProgramEntry = {
    .attributeId    = DemandResponseLoadControl::Attributes::NumberOfEventsPerProgram::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfTransitionsEntry = {
    .attributeId    = DemandResponseLoadControl::Attributes::NumberOfTransitions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDefaultRandomStartEntry = {
    .attributeId    = DemandResponseLoadControl::Attributes::DefaultRandomStart::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kDefaultRandomDurationEntry = {
    .attributeId    = DemandResponseLoadControl::Attributes::DefaultRandomDuration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kRegisterLoadControlProgramRequestEntry = {
    .commandId       = DemandResponseLoadControl::Commands::RegisterLoadControlProgramRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kUnregisterLoadControlProgramRequestEntry = {
    .commandId       = DemandResponseLoadControl::Commands::UnregisterLoadControlProgramRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kAddLoadControlEventRequestEntry = {
    .commandId       = DemandResponseLoadControl::Commands::AddLoadControlEventRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kRemoveLoadControlEventRequestEntry = {
    .commandId       = DemandResponseLoadControl::Commands::RemoveLoadControlEventRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kClearLoadControlEventsRequestEntry = {
    .commandId       = DemandResponseLoadControl::Commands::ClearLoadControlEventsRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace DemandResponseLoadControl
} // namespace clusters
} // namespace app
} // namespace chip
