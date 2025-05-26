// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TargetNavigator (cluster code: 1285/0x505)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TargetNavigator/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TargetNavigator {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace TargetList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(TargetList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace TargetList
namespace CurrentTarget {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentTarget::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentTarget

} // namespace Attributes

namespace Commands {
namespace NavigateTarget {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(NavigateTarget::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace NavigateTarget

} // namespace Commands
} // namespace TargetNavigator
} // namespace Clusters
} // namespace app
} // namespace chip
