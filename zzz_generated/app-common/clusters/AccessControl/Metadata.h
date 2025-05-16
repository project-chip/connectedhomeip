// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AccessControl (cluster code: 31/0x1F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AccessControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AccessControl {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace Acl {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Acl::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, Access::Privilege::kAdminister);
} // namespace Acl
namespace Extension {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Extension::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, Access::Privilege::kAdminister);
} // namespace Extension
namespace SubjectsPerAccessControlEntry {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SubjectsPerAccessControlEntry::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace SubjectsPerAccessControlEntry
namespace TargetsPerAccessControlEntry {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TargetsPerAccessControlEntry::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace TargetsPerAccessControlEntry
namespace AccessControlEntriesPerFabric {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AccessControlEntriesPerFabric::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AccessControlEntriesPerFabric
namespace CommissioningARL {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(CommissioningARL::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace CommissioningARL
namespace Arl {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Arl::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace Arl

} // namespace Attributes

namespace Commands {
namespace ReviewFabricRestrictions {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ReviewFabricRestrictions::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace ReviewFabricRestrictions

} // namespace Commands
} // namespace AccessControl
} // namespace Clusters
} // namespace app
} // namespace chip
