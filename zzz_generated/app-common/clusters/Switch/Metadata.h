// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Switch (cluster code: 59/0x3B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Switch/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Switch {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace NumberOfPositions {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfPositions::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NumberOfPositions
namespace CurrentPosition {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentPosition::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentPosition
namespace MultiPressMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MultiPressMax::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MultiPressMax

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Switch
} // namespace Clusters
} // namespace app
} // namespace chip
