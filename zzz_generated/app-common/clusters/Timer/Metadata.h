// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Timer (cluster code: 71/0x47)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Timer/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Timer {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace SetTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SetTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SetTime
namespace TimeRemaining {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TimeRemaining::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TimeRemaining
namespace TimerState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TimerState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TimerState

} // namespace Attributes

namespace Commands {
namespace SetTimer {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetTimer::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SetTimer
namespace ResetTimer {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ResetTimer::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ResetTimer
namespace AddTime {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddTime::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace AddTime
namespace ReduceTime {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ReduceTime::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ReduceTime

} // namespace Commands
} // namespace Timer
} // namespace Clusters
} // namespace app
} // namespace chip
