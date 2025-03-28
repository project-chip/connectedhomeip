// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DemandResponseLoadControl (cluster code: 150/0x96)
// based on src/controller/data_model/controller-clusters.matter

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
    .attributeId    = Attributes::LoadControlPrograms::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfLoadControlProgramsEntry = {
    .attributeId    = Attributes::NumberOfLoadControlPrograms::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kEventsEntry = {
    .attributeId    = Attributes::Events::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActiveEventsEntry = {
    .attributeId    = Attributes::ActiveEvents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfEventsPerProgramEntry = {
    .attributeId    = Attributes::NumberOfEventsPerProgram::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfTransitionsEntry = {
    .attributeId    = Attributes::NumberOfTransitions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDefaultRandomStartEntry = {
    .attributeId    = Attributes::DefaultRandomStart::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kDefaultRandomDurationEntry = {
    .attributeId    = Attributes::DefaultRandomDuration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kRegisterLoadControlProgramRequestEntry = {
    .commandId       = Commands::RegisterLoadControlProgramRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kUnregisterLoadControlProgramRequestEntry = {
    .commandId       = Commands::UnregisterLoadControlProgramRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kAddLoadControlEventRequestEntry = {
    .commandId       = Commands::AddLoadControlEventRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kRemoveLoadControlEventRequestEntry = {
    .commandId       = Commands::RemoveLoadControlEventRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kClearLoadControlEventsRequestEntry = {
    .commandId       = Commands::ClearLoadControlEventsRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace DemandResponseLoadControl
} // namespace clusters
} // namespace app
} // namespace chip
