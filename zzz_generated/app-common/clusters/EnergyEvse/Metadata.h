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
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = State::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace State
namespace SupplyState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SupplyState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupplyState
namespace FaultState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FaultState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace FaultState
namespace ChargingEnabledUntil {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ChargingEnabledUntil::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ChargingEnabledUntil
namespace DischargingEnabledUntil {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = DischargingEnabledUntil::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DischargingEnabledUntil
namespace CircuitCapacity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CircuitCapacity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CircuitCapacity
namespace MinimumChargeCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinimumChargeCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinimumChargeCurrent
namespace MaximumChargeCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaximumChargeCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaximumChargeCurrent
namespace MaximumDischargeCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaximumDischargeCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaximumDischargeCurrent
namespace UserMaximumChargeCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = UserMaximumChargeCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace UserMaximumChargeCurrent
namespace RandomizationDelayWindow {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RandomizationDelayWindow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace RandomizationDelayWindow
namespace NextChargeStartTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NextChargeStartTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextChargeStartTime
namespace NextChargeTargetTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NextChargeTargetTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextChargeTargetTime
namespace NextChargeRequiredEnergy {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NextChargeRequiredEnergy::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextChargeRequiredEnergy
namespace NextChargeTargetSoC {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NextChargeTargetSoC::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextChargeTargetSoC
namespace ApproximateEVEfficiency {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ApproximateEVEfficiency::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ApproximateEVEfficiency
namespace StateOfCharge {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = StateOfCharge::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace StateOfCharge
namespace BatteryCapacity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = BatteryCapacity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatteryCapacity
namespace VehicleID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = VehicleID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace VehicleID
namespace SessionID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SessionID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SessionID
namespace SessionDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SessionDuration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SessionDuration
namespace SessionEnergyCharged {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SessionEnergyCharged::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SessionEnergyCharged
namespace SessionEnergyDischarged {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SessionEnergyDischarged::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SessionEnergyDischarged

} // namespace Attributes

namespace Commands {
namespace Disable {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Disable::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Disable
namespace EnableCharging {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = EnableCharging::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace EnableCharging
namespace EnableDischarging {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = EnableDischarging::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace EnableDischarging
namespace StartDiagnostics {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = StartDiagnostics::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StartDiagnostics
namespace SetTargets {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetTargets::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SetTargets
namespace GetTargets {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GetTargets::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GetTargets
namespace ClearTargets {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ClearTargets::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ClearTargets

} // namespace Commands
} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
