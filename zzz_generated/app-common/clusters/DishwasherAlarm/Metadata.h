// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DishwasherAlarm (cluster code: 93/0x5D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/DishwasherAlarm/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DishwasherAlarm {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace Mask {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Mask::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Mask
namespace Latch {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Latch::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Latch
namespace State {
inline constexpr DataModel::AttributeEntry kMetadataEntry(State::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace State
namespace Supported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Supported::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Supported

} // namespace Attributes

namespace Commands {
namespace Reset {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Reset::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Reset
namespace ModifyEnabledAlarms {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ModifyEnabledAlarms::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ModifyEnabledAlarms

} // namespace Commands
} // namespace DishwasherAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
