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

namespace Attributes {} // namespace Attributes

namespace Commands {
namespace Sleep {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = LowPower::Commands::Sleep::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Sleep

} // namespace Commands
} // namespace LowPower
} // namespace Clusters
} // namespace app
} // namespace chip
