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
namespace Clusters {
namespace ElectricalPowerMeasurement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace PowerMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PowerMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PowerMode
namespace NumberOfMeasurementTypes {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfMeasurementTypes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfMeasurementTypes
namespace Accuracy {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Accuracy::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Accuracy
namespace Ranges {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Ranges::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Ranges
namespace Voltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Voltage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Voltage
namespace ActiveCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveCurrent
namespace ReactiveCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ReactiveCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ReactiveCurrent
namespace ApparentCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ApparentCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ApparentCurrent
namespace ActivePower {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActivePower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActivePower
namespace ReactivePower {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ReactivePower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ReactivePower
namespace ApparentPower {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ApparentPower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ApparentPower
namespace RMSVoltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RMSVoltage::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RMSVoltage
namespace RMSCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RMSCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RMSCurrent
namespace RMSPower {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RMSPower::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RMSPower
namespace Frequency {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Frequency::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Frequency
namespace HarmonicCurrents {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = HarmonicCurrents::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace HarmonicCurrents
namespace HarmonicPhases {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = HarmonicPhases::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace HarmonicPhases
namespace PowerFactor {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PowerFactor::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PowerFactor
namespace NeutralCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NeutralCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NeutralCurrent


} // namespace Attributes

namespace Commands {

} // namespace Commands
} // namespace ElectricalPowerMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
