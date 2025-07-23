// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster BooleanState (cluster code: 69/0x45)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/BooleanState/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace BooleanState {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
inline constexpr uint32_t kMetadataCount = 6;

namespace StateValue {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StateValue::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace StateValue

} // namespace Attributes

namespace Commands {
inline constexpr uint32_t kMetadataCount = 0;

} // namespace Commands
} // namespace BooleanState
} // namespace Clusters
} // namespace app
} // namespace chip
