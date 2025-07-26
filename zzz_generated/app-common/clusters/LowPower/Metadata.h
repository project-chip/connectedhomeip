// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LowPower (cluster code: 1288/0x508)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/LowPower/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LowPower {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
inline constexpr uint32_t kMetadataCount = 5;

} // namespace Attributes

namespace Commands {
inline constexpr uint32_t kMetadataCount = 1;

namespace Sleep {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Sleep::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Sleep

} // namespace Commands

namespace Events {} // namespace Events
} // namespace LowPower
} // namespace Clusters
} // namespace app
} // namespace chip
