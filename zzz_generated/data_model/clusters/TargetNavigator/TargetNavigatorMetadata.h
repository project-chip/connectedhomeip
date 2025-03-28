// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TargetNavigator (cluster code: 1285/0x505)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TargetNavigator/TargetNavigatorIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace TargetNavigator {
namespace Metadata {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kTargetListEntry = {
    .attributeId    = TargetNavigator::Attributes::TargetList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentTargetEntry = {
    .attributeId    = TargetNavigator::Attributes::CurrentTarget::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kNavigateTargetEntry = {
    .commandId       = TargetNavigator::Commands::NavigateTarget::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace TargetNavigator
} // namespace clusters
} // namespace app
} // namespace chip
