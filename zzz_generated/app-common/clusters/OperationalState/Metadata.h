// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OperationalState (cluster code: 96/0x60)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/OperationalState/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace PhaseList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(PhaseList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace PhaseList
namespace CurrentPhase {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentPhase::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentPhase
namespace CountdownTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CountdownTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CountdownTime
namespace OperationalStateList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(OperationalStateList::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace OperationalStateList
namespace OperationalState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OperationalState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OperationalState
namespace OperationalError {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OperationalError::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OperationalError

} // namespace Attributes

namespace Commands {
namespace Pause {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Pause::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Pause
namespace Stop {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Stop::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Stop
namespace Start {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Start::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Start
namespace Resume {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Resume::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Resume

} // namespace Commands
} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
