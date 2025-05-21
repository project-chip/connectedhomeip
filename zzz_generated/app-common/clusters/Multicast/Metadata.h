// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Multicast (cluster code: 100/0x64)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Multicast/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Multicast {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace Targets {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Targets::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, Access::Privilege::kManage);
} // namespace Targets

} // namespace Attributes

namespace Commands {
namespace SetTarget {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetTarget::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace SetTarget
namespace RemoveTarget {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RemoveTarget::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace RemoveTarget

} // namespace Commands
} // namespace Multicast
} // namespace Clusters
} // namespace app
} // namespace chip
