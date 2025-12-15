// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AmbientSensingUnion (cluster code: 1074/0x432)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AmbientSensingUnion/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AmbientSensingUnion {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace UnionName {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnionName::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace UnionName
namespace UnionHealth {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnionHealth::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UnionHealth
namespace UnionContributorList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(UnionContributorList::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace UnionContributorList
constexpr std::array<DataModel::AttributeEntry, 0> kMandatoryMetadata = {

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {
namespace UnionContributorListChange {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace UnionContributorListChange

} // namespace Events
} // namespace AmbientSensingUnion
} // namespace Clusters
} // namespace app
} // namespace chip
