// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster RvcOperationalState (cluster code: 97/0x61)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/RvcOperationalState/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace RvcOperationalState {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace PhaseList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PhaseList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PhaseList
namespace CurrentPhase {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentPhase::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentPhase
namespace CountdownTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CountdownTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CountdownTime
namespace OperationalStateList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OperationalStateList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OperationalStateList
namespace OperationalState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OperationalState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OperationalState
namespace OperationalError {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OperationalError::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OperationalError

} // namespace Attributes

namespace Commands {
namespace Pause {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RvcOperationalState::Commands::Pause::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Pause
namespace Resume {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RvcOperationalState::Commands::Resume::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Resume
namespace GoHome {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RvcOperationalState::Commands::GoHome::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GoHome

} // namespace Commands
} // namespace RvcOperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
