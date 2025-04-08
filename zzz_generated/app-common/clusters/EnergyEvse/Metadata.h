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
    .attributeId    = EnergyEvse::Attributes::State::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace State
namespace SupplyState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::SupplyState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupplyState
namespace FaultState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::FaultState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace FaultState
namespace ChargingEnabledUntil {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::ChargingEnabledUntil::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ChargingEnabledUntil
namespace DischargingEnabledUntil {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::DischargingEnabledUntil::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DischargingEnabledUntil
namespace CircuitCapacity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::CircuitCapacity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CircuitCapacity
namespace MinimumChargeCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::MinimumChargeCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinimumChargeCurrent
namespace MaximumChargeCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::MaximumChargeCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaximumChargeCurrent
namespace MaximumDischargeCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::MaximumDischargeCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaximumDischargeCurrent
namespace UserMaximumChargeCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::UserMaximumChargeCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace UserMaximumChargeCurrent
namespace RandomizationDelayWindow {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::RandomizationDelayWindow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace RandomizationDelayWindow
namespace NextChargeStartTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::NextChargeStartTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextChargeStartTime
namespace NextChargeTargetTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::NextChargeTargetTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextChargeTargetTime
namespace NextChargeRequiredEnergy {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::NextChargeRequiredEnergy::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextChargeRequiredEnergy
namespace NextChargeTargetSoC {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::NextChargeTargetSoC::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NextChargeTargetSoC
namespace ApproximateEVEfficiency {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::ApproximateEVEfficiency::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ApproximateEVEfficiency
namespace StateOfCharge {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::StateOfCharge::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace StateOfCharge
namespace BatteryCapacity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::BatteryCapacity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace BatteryCapacity
namespace VehicleID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::VehicleID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace VehicleID
namespace SessionID {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::SessionID::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SessionID
namespace SessionDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::SessionDuration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SessionDuration
namespace SessionEnergyCharged {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::SessionEnergyCharged::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SessionEnergyCharged
namespace SessionEnergyDischarged {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EnergyEvse::Attributes::SessionEnergyDischarged::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SessionEnergyDischarged


} // namespace Attributes

namespace Commands {
namespace Disable {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = EnergyEvse::Commands::Disable::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Disable
namespace EnableCharging {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = EnergyEvse::Commands::EnableCharging::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace EnableCharging
namespace EnableDischarging {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = EnergyEvse::Commands::EnableDischarging::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace EnableDischarging
namespace StartDiagnostics {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = EnergyEvse::Commands::StartDiagnostics::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace StartDiagnostics
namespace SetTargets {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = EnergyEvse::Commands::SetTargets::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SetTargets
namespace GetTargets {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = EnergyEvse::Commands::GetTargets::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GetTargets
namespace ClearTargets {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = EnergyEvse::Commands::ClearTargets::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace ClearTargets

} // namespace Commands
} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
