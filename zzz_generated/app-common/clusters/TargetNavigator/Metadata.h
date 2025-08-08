// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TargetNavigator (cluster code: 1285/0x505)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
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
constexpr std::array<DataModel::AttributeEntry, 1> kMandatoryMetadata = {
    TargetList::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace NavigateTarget {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(NavigateTarget::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace NavigateTarget

} // namespace Commands

namespace Events {
namespace TargetUpdated {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace TargetUpdated

} // namespace Events
} // namespace TargetNavigator
} // namespace Clusters
} // namespace app
} // namespace chip
