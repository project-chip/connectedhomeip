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
inline constexpr DataModel::AttributeEntry kMetadataEntry(CountdownTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CountdownTime
namespace MainState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MainState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MainState
namespace CurrentErrorList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(CurrentErrorList::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace CurrentErrorList
namespace OverallState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OverallState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OverallState
namespace OverallTarget {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OverallTarget::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OverallTarget

} // namespace Attributes

namespace Commands {
namespace Stop {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Stop::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Stop
namespace MoveTo {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(MoveTo::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace MoveTo
namespace Calibrate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Calibrate::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace Calibrate

} // namespace Commands
} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
