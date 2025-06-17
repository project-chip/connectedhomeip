// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster EnergyEvse (cluster code: 153/0x99)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/EnergyEvse/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

inline constexpr uint32_t kRevision = 3;

namespace Attributes {
namespace State {
inline constexpr DataModel::AttributeEntry kMetadataEntry(State::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace State
namespace SupplyState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SupplyState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SupplyState
namespace FaultState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(FaultState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace FaultState
namespace ChargingEnabledUntil {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ChargingEnabledUntil::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ChargingEnabledUntil
namespace DischargingEnabledUntil {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DischargingEnabledUntil::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace DischargingEnabledUntil
namespace CircuitCapacity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CircuitCapacity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CircuitCapacity
namespace MinimumChargeCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinimumChargeCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinimumChargeCurrent
namespace MaximumChargeCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaximumChargeCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaximumChargeCurrent
namespace MaximumDischargeCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaximumDischargeCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaximumDischargeCurrent
namespace UserMaximumChargeCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UserMaximumChargeCurrent::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace UserMaximumChargeCurrent
namespace RandomizationDelayWindow {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RandomizationDelayWindow::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace RandomizationDelayWindow
namespace NextChargeStartTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NextChargeStartTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NextChargeStartTime
namespace NextChargeTargetTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NextChargeTargetTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NextChargeTargetTime
namespace NextChargeRequiredEnergy {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NextChargeRequiredEnergy::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NextChargeRequiredEnergy
namespace NextChargeTargetSoC {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NextChargeTargetSoC::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NextChargeTargetSoC
namespace ApproximateEVEfficiency {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ApproximateEVEfficiency::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ApproximateEVEfficiency
namespace StateOfCharge {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StateOfCharge::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace StateOfCharge
namespace BatteryCapacity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BatteryCapacity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BatteryCapacity
namespace VehicleID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(VehicleID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace VehicleID
namespace SessionID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SessionID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SessionID
namespace SessionDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SessionDuration::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SessionDuration
namespace SessionEnergyCharged {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SessionEnergyCharged::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SessionEnergyCharged
namespace SessionEnergyDischarged {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SessionEnergyDischarged::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SessionEnergyDischarged

} // namespace Attributes

namespace Commands {
namespace Disable {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(Disable::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kOperate);
} // namespace Disable
namespace EnableCharging {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(EnableCharging::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kOperate);
} // namespace EnableCharging
namespace EnableDischarging {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(EnableDischarging::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kOperate);
} // namespace EnableDischarging
namespace StartDiagnostics {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(StartDiagnostics::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kOperate);
} // namespace StartDiagnostics
namespace SetTargets {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetTargets::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kOperate);
} // namespace SetTargets
namespace GetTargets {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(GetTargets::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kOperate);
} // namespace GetTargets
namespace ClearTargets {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ClearTargets::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kOperate);
} // namespace ClearTargets

} // namespace Commands
} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
