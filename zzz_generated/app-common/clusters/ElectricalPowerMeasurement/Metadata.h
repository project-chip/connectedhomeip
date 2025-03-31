// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalPowerMeasurement (cluster code: 144/0x90)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ElectricalPowerMeasurement/Ids.h>

namespace chip {
namespace app {
namespace clusters {
namespace ElectricalPowerMeasurement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace PowerMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::PowerMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PowerMode
namespace NumberOfMeasurementTypes {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::NumberOfMeasurementTypes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfMeasurementTypes
namespace Accuracy {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::Accuracy::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Accuracy
namespace Ranges {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::Ranges::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Ranges
namespace Voltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::Voltage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Voltage
namespace ActiveCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::ActiveCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveCurrent
namespace ReactiveCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::ReactiveCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ReactiveCurrent
namespace ApparentCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::ApparentCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ApparentCurrent
namespace ActivePower {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::ActivePower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActivePower
namespace ReactivePower {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::ReactivePower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ReactivePower
namespace ApparentPower {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::ApparentPower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ApparentPower
namespace RMSVoltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::RMSVoltage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RMSVoltage
namespace RMSCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::RMSCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RMSCurrent
namespace RMSPower {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::RMSPower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RMSPower
namespace Frequency {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::Frequency::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Frequency
namespace HarmonicCurrents {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::HarmonicCurrents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace HarmonicCurrents
namespace HarmonicPhases {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::HarmonicPhases::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace HarmonicPhases
namespace PowerFactor {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::PowerFactor::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PowerFactor
namespace NeutralCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ElectricalPowerMeasurement::Attributes::NeutralCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NeutralCurrent

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace ElectricalPowerMeasurement
} // namespace clusters
} // namespace app
} // namespace chip
