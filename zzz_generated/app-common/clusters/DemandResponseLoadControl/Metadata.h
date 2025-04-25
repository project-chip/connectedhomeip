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
inline constexpr DataModel::AttributeEntry kMetadataEntry{
    LoadControlPrograms::Id, BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    Access::Privilege::kView, std::nullopt
};
} // namespace LoadControlPrograms
namespace NumberOfLoadControlPrograms {
inline constexpr DataModel::AttributeEntry kMetadataEntry{ NumberOfLoadControlPrograms::Id,
                                                           BitFlags<DataModel::AttributeQualityFlags>{}, Access::Privilege::kView,
                                                           std::nullopt };
} // namespace NumberOfLoadControlPrograms
namespace Events {
inline constexpr DataModel::AttributeEntry kMetadataEntry{
    Events::Id, BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    Access::Privilege::kView, std::nullopt
};
} // namespace Events
namespace ActiveEvents {
inline constexpr DataModel::AttributeEntry kMetadataEntry{
    ActiveEvents::Id, BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    Access::Privilege::kView, std::nullopt
};
} // namespace ActiveEvents
namespace NumberOfEventsPerProgram {
inline constexpr DataModel::AttributeEntry kMetadataEntry{ NumberOfEventsPerProgram::Id,
                                                           BitFlags<DataModel::AttributeQualityFlags>{}, Access::Privilege::kView,
                                                           std::nullopt };
} // namespace NumberOfEventsPerProgram
namespace NumberOfTransitions {
inline constexpr DataModel::AttributeEntry kMetadataEntry{ NumberOfTransitions::Id, BitFlags<DataModel::AttributeQualityFlags>{},
                                                           Access::Privilege::kView, std::nullopt };
} // namespace NumberOfTransitions
namespace DefaultRandomStart {
inline constexpr DataModel::AttributeEntry kMetadataEntry{ DefaultRandomStart::Id, BitFlags<DataModel::AttributeQualityFlags>{},
                                                           Access::Privilege::kView, Access::Privilege::kManage };
} // namespace DefaultRandomStart
namespace DefaultRandomDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry{ DefaultRandomDuration::Id, BitFlags<DataModel::AttributeQualityFlags>{},
                                                           Access::Privilege::kView, Access::Privilege::kManage };
} // namespace DefaultRandomDuration

} // namespace Attributes

namespace Commands {
namespace RegisterLoadControlProgramRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{
    DemandResponseLoadControl::Commands::RegisterLoadControlProgramRequest::Id, BitFlags<DataModel::CommandQualityFlags>{},
    Access::Privilege::kOperate
};
} // namespace RegisterLoadControlProgramRequest
namespace UnregisterLoadControlProgramRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{
    DemandResponseLoadControl::Commands::UnregisterLoadControlProgramRequest::Id, BitFlags<DataModel::CommandQualityFlags>{},
    Access::Privilege::kOperate
};
} // namespace UnregisterLoadControlProgramRequest
namespace AddLoadControlEventRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{
    DemandResponseLoadControl::Commands::AddLoadControlEventRequest::Id, BitFlags<DataModel::CommandQualityFlags>{},
    Access::Privilege::kOperate
};
} // namespace AddLoadControlEventRequest
namespace RemoveLoadControlEventRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{
    DemandResponseLoadControl::Commands::RemoveLoadControlEventRequest::Id, BitFlags<DataModel::CommandQualityFlags>{},
    Access::Privilege::kOperate
};
} // namespace RemoveLoadControlEventRequest
namespace ClearLoadControlEventsRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry{
    DemandResponseLoadControl::Commands::ClearLoadControlEventsRequest::Id, BitFlags<DataModel::CommandQualityFlags>{},
    Access::Privilege::kOperate
};
} // namespace ClearLoadControlEventsRequest

} // namespace Commands
} // namespace DemandResponseLoadControl
} // namespace Clusters
} // namespace app
} // namespace chip
