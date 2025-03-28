// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster BasicInformation (cluster code: 40/0x28)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/BasicInformation/BasicInformationIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace BasicInformation {
namespace Metadata {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kDataModelRevisionEntry = {
    .attributeId    = BasicInformation::Attributes::DataModelRevision::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kVendorNameEntry = {
    .attributeId    = BasicInformation::Attributes::VendorName::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kVendorIDEntry = {
    .attributeId    = BasicInformation::Attributes::VendorID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kProductNameEntry = {
    .attributeId    = BasicInformation::Attributes::ProductName::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kProductIDEntry = {
    .attributeId    = BasicInformation::Attributes::ProductID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNodeLabelEntry = {
    .attributeId    = BasicInformation::Attributes::NodeLabel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kLocationEntry = {
    .attributeId    = BasicInformation::Attributes::Location::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AttributeEntry kHardwareVersionEntry = {
    .attributeId    = BasicInformation::Attributes::HardwareVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kHardwareVersionStringEntry = {
    .attributeId    = BasicInformation::Attributes::HardwareVersionString::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSoftwareVersionEntry = {
    .attributeId    = BasicInformation::Attributes::SoftwareVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSoftwareVersionStringEntry = {
    .attributeId    = BasicInformation::Attributes::SoftwareVersionString::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kManufacturingDateEntry = {
    .attributeId    = BasicInformation::Attributes::ManufacturingDate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPartNumberEntry = {
    .attributeId    = BasicInformation::Attributes::PartNumber::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kProductURLEntry = {
    .attributeId    = BasicInformation::Attributes::ProductURL::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kProductLabelEntry = {
    .attributeId    = BasicInformation::Attributes::ProductLabel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSerialNumberEntry = {
    .attributeId    = BasicInformation::Attributes::SerialNumber::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLocalConfigDisabledEntry = {
    .attributeId    = BasicInformation::Attributes::LocalConfigDisabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kReachableEntry = {
    .attributeId    = BasicInformation::Attributes::Reachable::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kUniqueIDEntry = {
    .attributeId    = BasicInformation::Attributes::UniqueID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCapabilityMinimaEntry = {
    .attributeId    = BasicInformation::Attributes::CapabilityMinima::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kProductAppearanceEntry = {
    .attributeId    = BasicInformation::Attributes::ProductAppearance::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSpecificationVersionEntry = {
    .attributeId    = BasicInformation::Attributes::SpecificationVersion::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxPathsPerInvokeEntry = {
    .attributeId    = BasicInformation::Attributes::MaxPathsPerInvoke::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kMfgSpecificPingEntry = {
    .commandId       = BasicInformation::Commands::MfgSpecificPing::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace BasicInformation
} // namespace clusters
} // namespace app
} // namespace chip
