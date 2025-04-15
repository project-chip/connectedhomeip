// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DemandResponseLoadControl (cluster code: 150/0x96)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/DemandResponseLoadControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DemandResponseLoadControl {

inline constexpr uint32_t kRevision = 4;

namespace Attributes {
namespace LoadControlPrograms {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LoadControlPrograms::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LoadControlPrograms
namespace NumberOfLoadControlPrograms {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfLoadControlPrograms::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfLoadControlPrograms
namespace Events {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Events::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Events
namespace ActiveEvents {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveEvents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveEvents
namespace NumberOfEventsPerProgram {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfEventsPerProgram::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfEventsPerProgram
namespace NumberOfTransitions {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfTransitions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfTransitions
namespace DefaultRandomStart {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DefaultRandomStart::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace DefaultRandomStart
namespace DefaultRandomDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DefaultRandomDuration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace DefaultRandomDuration

} // namespace Attributes

namespace Commands {
namespace RegisterLoadControlProgramRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = DemandResponseLoadControl::Commands::RegisterLoadControlProgramRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace RegisterLoadControlProgramRequest
namespace UnregisterLoadControlProgramRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = DemandResponseLoadControl::Commands::UnregisterLoadControlProgramRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace UnregisterLoadControlProgramRequest
namespace AddLoadControlEventRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = DemandResponseLoadControl::Commands::AddLoadControlEventRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace AddLoadControlEventRequest
namespace RemoveLoadControlEventRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = DemandResponseLoadControl::Commands::RemoveLoadControlEventRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace RemoveLoadControlEventRequest
namespace ClearLoadControlEventsRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = DemandResponseLoadControl::Commands::ClearLoadControlEventsRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ClearLoadControlEventsRequest

} // namespace Commands
} // namespace DemandResponseLoadControl
} // namespace Clusters
} // namespace app
} // namespace chip
