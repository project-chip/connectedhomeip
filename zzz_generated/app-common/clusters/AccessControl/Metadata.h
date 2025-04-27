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
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Acl::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = Access::Privilege::kAdminister,
};
} // namespace Acl
namespace Extension {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Extension::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = Access::Privilege::kAdminister,
};
} // namespace Extension
namespace SubjectsPerAccessControlEntry {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SubjectsPerAccessControlEntry::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SubjectsPerAccessControlEntry
namespace TargetsPerAccessControlEntry {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TargetsPerAccessControlEntry::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TargetsPerAccessControlEntry
namespace AccessControlEntriesPerFabric {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AccessControlEntriesPerFabric::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AccessControlEntriesPerFabric
namespace CommissioningARL {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommissioningARL::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CommissioningARL
namespace Arl {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Arl::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Arl

} // namespace Attributes

namespace Commands {
namespace ReviewFabricRestrictions {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ReviewFabricRestrictions::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ReviewFabricRestrictions

} // namespace Commands
} // namespace AccessControl
} // namespace Clusters
} // namespace app
} // namespace chip
