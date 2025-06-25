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

inline constexpr uint32_t kRevision = 3;

namespace Attributes {
namespace PowerMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PowerMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PowerMode
namespace NumberOfMeasurementTypes {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfMeasurementTypes::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NumberOfMeasurementTypes
namespace Accuracy {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Accuracy::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace Accuracy
namespace Ranges {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Ranges::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace Ranges
namespace Voltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Voltage::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Voltage
namespace ActiveCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ActiveCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ActiveCurrent
namespace ReactiveCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ReactiveCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ReactiveCurrent
namespace ApparentCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ApparentCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ApparentCurrent
namespace ActivePower {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ActivePower::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ActivePower
namespace ReactivePower {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ReactivePower::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ReactivePower
namespace ApparentPower {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ApparentPower::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ApparentPower
namespace RMSVoltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RMSVoltage::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RMSVoltage
namespace RMSCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RMSCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RMSCurrent
namespace RMSPower {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RMSPower::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace RMSPower
namespace Frequency {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Frequency::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Frequency
namespace HarmonicCurrents {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(HarmonicCurrents::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace HarmonicCurrents
namespace HarmonicPhases {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(HarmonicPhases::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace HarmonicPhases
namespace PowerFactor {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PowerFactor::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PowerFactor
namespace NeutralCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NeutralCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NeutralCurrent

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace ElectricalPowerMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
