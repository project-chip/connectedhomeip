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
inline constexpr DataModel::AttributeEntry kMetadataEntry(Status::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Status
namespace Order {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Order::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Order
namespace Description {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Description::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Description
namespace WiredAssessedInputVoltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WiredAssessedInputVoltage::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace WiredAssessedInputVoltage
namespace WiredAssessedInputFrequency {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WiredAssessedInputFrequency::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace WiredAssessedInputFrequency
namespace WiredCurrentType {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WiredCurrentType::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace WiredCurrentType
namespace WiredAssessedCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WiredAssessedCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace WiredAssessedCurrent
namespace WiredNominalVoltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WiredNominalVoltage::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace WiredNominalVoltage
namespace WiredMaximumCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WiredMaximumCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace WiredMaximumCurrent
namespace WiredPresent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WiredPresent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace WiredPresent
namespace ActiveWiredFaults {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ActiveWiredFaults::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ActiveWiredFaults
namespace BatVoltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatVoltage::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatVoltage
namespace BatPercentRemaining {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatPercentRemaining::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatPercentRemaining
namespace BatTimeRemaining {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatTimeRemaining::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatTimeRemaining
namespace BatChargeLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatChargeLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatChargeLevel
namespace BatReplacementNeeded {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatReplacementNeeded::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatReplacementNeeded
namespace BatReplaceability {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatReplaceability::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatReplaceability
namespace BatPresent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatPresent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatPresent
namespace ActiveBatFaults {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ActiveBatFaults::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ActiveBatFaults
namespace BatReplacementDescription {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatReplacementDescription::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace BatReplacementDescription
namespace BatCommonDesignation {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatCommonDesignation::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatCommonDesignation
namespace BatANSIDesignation {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatANSIDesignation::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatANSIDesignation
namespace BatIECDesignation {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatIECDesignation::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatIECDesignation
namespace BatApprovedChemistry {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatApprovedChemistry::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatApprovedChemistry
namespace BatCapacity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatCapacity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatCapacity
namespace BatQuantity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatQuantity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatQuantity
namespace BatChargeState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatChargeState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatChargeState
namespace BatTimeToFullCharge {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatTimeToFullCharge::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatTimeToFullCharge
namespace BatFunctionalWhileCharging {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatFunctionalWhileCharging::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace BatFunctionalWhileCharging
namespace BatChargingCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatChargingCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatChargingCurrent
namespace ActiveBatChargeFaults {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ActiveBatChargeFaults::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ActiveBatChargeFaults
namespace EndpointList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(EndpointList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace EndpointList

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace PowerSource
} // namespace Clusters
} // namespace app
} // namespace chip
