// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AccessControl (cluster code: 31/0x1F)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AccessControl/AccessControlIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace AccessControl {
namespace Metadata {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kAclEntry = {
    .attributeId    = Attributes::Acl::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AttributeEntry kExtensionEntry = {
    .attributeId    = Attributes::Extension::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AttributeEntry kSubjectsPerAccessControlEntryEntry = {
    .attributeId    = Attributes::SubjectsPerAccessControlEntry::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTargetsPerAccessControlEntryEntry = {
    .attributeId    = Attributes::TargetsPerAccessControlEntry::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAccessControlEntriesPerFabricEntry = {
    .attributeId    = Attributes::AccessControlEntriesPerFabric::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCommissioningARLEntry = {
    .attributeId    = Attributes::CommissioningARL::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kArlEntry = {
    .attributeId    = Attributes::Arl::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kReviewFabricRestrictionsEntry = {
    .commandId       = Commands::ReviewFabricRestrictions::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};

} // namespace Commands
} // namespace Metadata
} // namespace AccessControl
} // namespace clusters
} // namespace app
} // namespace chip
