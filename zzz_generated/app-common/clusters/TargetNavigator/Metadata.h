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
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::TargetList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TargetList
namespace CurrentTarget {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CurrentTarget::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentTarget

} // namespace Attributes

namespace Commands {
namespace NavigateTarget {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::NavigateTarget::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace NavigateTarget

} // namespace Commands
} // namespace TargetNavigator
} // namespace Clusters
} // namespace app
} // namespace chip
