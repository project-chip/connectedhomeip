// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster BasicInformation (cluster code: 40/0x28)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/BasicInformation/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace BasicInformation {

inline constexpr uint32_t kRevision = 5;

namespace Attributes {
namespace DataModelRevision {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DataModelRevision::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DataModelRevision
namespace VendorName {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = VendorName::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace VendorName
namespace VendorID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = VendorID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace VendorID
namespace ProductName {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ProductName::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ProductName
namespace ProductID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ProductID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ProductID
namespace NodeLabel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NodeLabel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace NodeLabel
namespace Location {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Location::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
} // namespace Location
namespace HardwareVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = HardwareVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace HardwareVersion
namespace HardwareVersionString {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = HardwareVersionString::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace HardwareVersionString
namespace SoftwareVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SoftwareVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SoftwareVersion
namespace SoftwareVersionString {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SoftwareVersionString::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SoftwareVersionString
namespace ManufacturingDate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ManufacturingDate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ManufacturingDate
namespace PartNumber {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PartNumber::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PartNumber
namespace ProductURL {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ProductURL::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ProductURL
namespace ProductLabel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ProductLabel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ProductLabel
namespace SerialNumber {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SerialNumber::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SerialNumber
namespace LocalConfigDisabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LocalConfigDisabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LocalConfigDisabled
namespace Reachable {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Reachable::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Reachable
namespace UniqueID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = UniqueID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace UniqueID
namespace CapabilityMinima {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CapabilityMinima::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CapabilityMinima
namespace ProductAppearance {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ProductAppearance::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ProductAppearance
namespace SpecificationVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SpecificationVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SpecificationVersion
namespace MaxPathsPerInvoke {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxPathsPerInvoke::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxPathsPerInvoke
namespace ConfigurationVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ConfigurationVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ConfigurationVersion

} // namespace Attributes

namespace Commands {
namespace MfgSpecificPing {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = MfgSpecificPing::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace MfgSpecificPing

} // namespace Commands
} // namespace BasicInformation
} // namespace Clusters
} // namespace app
} // namespace chip
