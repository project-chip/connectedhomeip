// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PowerSource (cluster code: 47/0x2F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/PowerSource/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerSource {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace Status {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Status::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Status
namespace Order {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Order::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Order
namespace Description {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Description::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Description
namespace WiredAssessedInputVoltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::WiredAssessedInputVoltage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiredAssessedInputVoltage
namespace WiredAssessedInputFrequency {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::WiredAssessedInputFrequency::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiredAssessedInputFrequency
namespace WiredCurrentType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::WiredCurrentType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiredCurrentType
namespace WiredAssessedCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::WiredAssessedCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiredAssessedCurrent
namespace WiredNominalVoltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::WiredNominalVoltage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiredNominalVoltage
namespace WiredMaximumCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::WiredMaximumCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiredMaximumCurrent
namespace WiredPresent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::WiredPresent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiredPresent
namespace ActiveWiredFaults {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ActiveWiredFaults::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveWiredFaults
namespace BatVoltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatVoltage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatVoltage
namespace BatPercentRemaining {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatPercentRemaining::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatPercentRemaining
namespace BatTimeRemaining {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatTimeRemaining::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatTimeRemaining
namespace BatChargeLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatChargeLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatChargeLevel
namespace BatReplacementNeeded {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatReplacementNeeded::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatReplacementNeeded
namespace BatReplaceability {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatReplaceability::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatReplaceability
namespace BatPresent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatPresent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatPresent
namespace ActiveBatFaults {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ActiveBatFaults::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveBatFaults
namespace BatReplacementDescription {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatReplacementDescription::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatReplacementDescription
namespace BatCommonDesignation {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatCommonDesignation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatCommonDesignation
namespace BatANSIDesignation {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatANSIDesignation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatANSIDesignation
namespace BatIECDesignation {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatIECDesignation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatIECDesignation
namespace BatApprovedChemistry {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatApprovedChemistry::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatApprovedChemistry
namespace BatCapacity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatCapacity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatCapacity
namespace BatQuantity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatQuantity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatQuantity
namespace BatChargeState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatChargeState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatChargeState
namespace BatTimeToFullCharge {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatTimeToFullCharge::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatTimeToFullCharge
namespace BatFunctionalWhileCharging {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatFunctionalWhileCharging::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatFunctionalWhileCharging
namespace BatChargingCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::BatChargingCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatChargingCurrent
namespace ActiveBatChargeFaults {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ActiveBatChargeFaults::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveBatChargeFaults
namespace EndpointList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::EndpointList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace EndpointList

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace PowerSource
} // namespace Clusters
} // namespace app
} // namespace chip
