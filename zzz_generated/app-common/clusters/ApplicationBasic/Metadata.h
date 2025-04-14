// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ApplicationBasic (cluster code: 1293/0x50D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ApplicationBasic/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationBasic {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace VendorName {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::VendorName::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace VendorName
namespace VendorID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::VendorID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace VendorID
namespace ApplicationName {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ApplicationName::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ApplicationName
namespace ProductID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ProductID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ProductID
namespace Application {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Application::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Application
namespace Status {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Status::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Status
namespace ApplicationVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ApplicationVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ApplicationVersion
namespace AllowedVendorList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::AllowedVendorList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace AllowedVendorList

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace ApplicationBasic
} // namespace Clusters
} // namespace app
} // namespace chip
