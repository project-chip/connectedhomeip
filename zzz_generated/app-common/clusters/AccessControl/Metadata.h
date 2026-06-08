// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AccessControl (cluster code: 31/0x1F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AccessControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AccessControl {

inline constexpr uint32_t kRevision = 3;

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
namespace AuxiliaryACL {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AuxiliaryACL::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace AuxiliaryACL
constexpr std::array<DataModel::AttributeEntry, 4> kMandatoryMetadata = {
    Acl::kMetadataEntry,
    SubjectsPerAccessControlEntry::kMetadataEntry,
    TargetsPerAccessControlEntry::kMetadataEntry,
    AccessControlEntriesPerFabric::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace ReviewFabricRestrictions {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ReviewFabricRestrictions::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace ReviewFabricRestrictions

} // namespace Commands

namespace Events {
namespace AccessControlEntryChanged {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kAdminister };
} // namespace AccessControlEntryChanged
namespace AccessControlExtensionChanged {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kAdminister };
} // namespace AccessControlExtensionChanged
namespace FabricRestrictionReviewUpdate {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kAdminister };
} // namespace FabricRestrictionReviewUpdate
namespace AuxiliaryAccessUpdated {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kAdminister };
} // namespace AuxiliaryAccessUpdated

} // namespace Events
} // namespace AccessControl
} // namespace Clusters
} // namespace app
} // namespace chip
