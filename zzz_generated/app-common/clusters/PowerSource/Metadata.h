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
    .attributeId    = Status::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Status
namespace Order {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Order::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Order
namespace Description {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Description::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Description
namespace WiredAssessedInputVoltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = WiredAssessedInputVoltage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiredAssessedInputVoltage
namespace WiredAssessedInputFrequency {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = WiredAssessedInputFrequency::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiredAssessedInputFrequency
namespace WiredCurrentType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = WiredCurrentType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiredCurrentType
namespace WiredAssessedCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = WiredAssessedCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiredAssessedCurrent
namespace WiredNominalVoltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = WiredNominalVoltage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiredNominalVoltage
namespace WiredMaximumCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = WiredMaximumCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiredMaximumCurrent
namespace WiredPresent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = WiredPresent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WiredPresent
namespace ActiveWiredFaults {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveWiredFaults::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveWiredFaults
namespace BatVoltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatVoltage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatVoltage
namespace BatPercentRemaining {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatPercentRemaining::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatPercentRemaining
namespace BatTimeRemaining {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatTimeRemaining::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatTimeRemaining
namespace BatChargeLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatChargeLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatChargeLevel
namespace BatReplacementNeeded {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatReplacementNeeded::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatReplacementNeeded
namespace BatReplaceability {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatReplaceability::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatReplaceability
namespace BatPresent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatPresent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatPresent
namespace ActiveBatFaults {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveBatFaults::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveBatFaults
namespace BatReplacementDescription {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatReplacementDescription::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatReplacementDescription
namespace BatCommonDesignation {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatCommonDesignation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatCommonDesignation
namespace BatANSIDesignation {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatANSIDesignation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatANSIDesignation
namespace BatIECDesignation {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatIECDesignation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatIECDesignation
namespace BatApprovedChemistry {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatApprovedChemistry::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatApprovedChemistry
namespace BatCapacity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatCapacity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatCapacity
namespace BatQuantity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatQuantity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatQuantity
namespace BatChargeState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatChargeState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatChargeState
namespace BatTimeToFullCharge {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatTimeToFullCharge::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatTimeToFullCharge
namespace BatFunctionalWhileCharging {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatFunctionalWhileCharging::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatFunctionalWhileCharging
namespace BatChargingCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatChargingCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatChargingCurrent
namespace ActiveBatChargeFaults {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveBatChargeFaults::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveBatChargeFaults
namespace EndpointList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EndpointList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace EndpointList


} // namespace Attributes

namespace Commands {

} // namespace Commands
} // namespace PowerSource
} // namespace Clusters
} // namespace app
} // namespace chip
