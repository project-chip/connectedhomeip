// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ClosureControl (cluster code: 260/0x104)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ClosureControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace CountdownTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CountdownTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CountdownTime
namespace MainState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::MainState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MainState
namespace CurrentErrorList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CurrentErrorList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentErrorList
namespace OverallState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::OverallState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OverallState
namespace OverallTarget {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::OverallTarget::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OverallTarget

} // namespace Attributes

namespace Commands {
namespace Stop {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::Stop::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Stop
namespace MoveTo {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::MoveTo::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MoveTo
namespace Calibrate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::Calibrate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace Calibrate

} // namespace Commands
} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
