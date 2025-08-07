// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ApplicationBasic (cluster code: 1293/0x50D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
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
inline constexpr DataModel::AttributeEntry kMetadataEntry(VendorName::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace VendorName
namespace VendorID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(VendorID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace VendorID
namespace ApplicationName {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ApplicationName::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ApplicationName
namespace ProductID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ProductID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ProductID
namespace Application {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Application::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Application
namespace Status {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Status::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Status
namespace ApplicationVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ApplicationVersion::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ApplicationVersion
namespace AllowedVendorList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AllowedVendorList::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace AllowedVendorList
constexpr std::array<DataModel::AttributeEntry, 5> kMandatoryMetadata = {
    ApplicationName::kMetadataEntry,    Application::kMetadataEntry,       Status::kMetadataEntry,
    ApplicationVersion::kMetadataEntry, AllowedVendorList::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace ApplicationBasic
} // namespace Clusters
} // namespace app
} // namespace chip
