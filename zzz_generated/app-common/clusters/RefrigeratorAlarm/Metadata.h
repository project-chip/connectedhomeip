// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster RefrigeratorAlarm (cluster code: 87/0x57)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/RefrigeratorAlarm/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace RefrigeratorAlarm {

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

} // namespace Commands

namespace Events {
namespace Notify {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace Notify

} // namespace Events
} // namespace RefrigeratorAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
