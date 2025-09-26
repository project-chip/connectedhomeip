// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster EnergyEvse (cluster code: 153/0x99)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/EnergyEvse/Ids.h>
#include <clusters/EnergyEvse/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::EnergyEvse::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::EnergyEvse::Attributes;
        switch (attributeId)
        {
        case State::Id:
            return State::kMetadataEntry;
        case SupplyState::Id:
            return SupplyState::kMetadataEntry;
        case FaultState::Id:
            return FaultState::kMetadataEntry;
        case ChargingEnabledUntil::Id:
            return ChargingEnabledUntil::kMetadataEntry;
        case DischargingEnabledUntil::Id:
            return DischargingEnabledUntil::kMetadataEntry;
        case CircuitCapacity::Id:
            return CircuitCapacity::kMetadataEntry;
        case MinimumChargeCurrent::Id:
            return MinimumChargeCurrent::kMetadataEntry;
        case MaximumChargeCurrent::Id:
            return MaximumChargeCurrent::kMetadataEntry;
        case MaximumDischargeCurrent::Id:
            return MaximumDischargeCurrent::kMetadataEntry;
        case UserMaximumChargeCurrent::Id:
            return UserMaximumChargeCurrent::kMetadataEntry;
        case RandomizationDelayWindow::Id:
            return RandomizationDelayWindow::kMetadataEntry;
        case NextChargeStartTime::Id:
            return NextChargeStartTime::kMetadataEntry;
        case NextChargeTargetTime::Id:
            return NextChargeTargetTime::kMetadataEntry;
        case NextChargeRequiredEnergy::Id:
            return NextChargeRequiredEnergy::kMetadataEntry;
        case NextChargeTargetSoC::Id:
            return NextChargeTargetSoC::kMetadataEntry;
        case ApproximateEVEfficiency::Id:
            return ApproximateEVEfficiency::kMetadataEntry;
        case StateOfCharge::Id:
            return StateOfCharge::kMetadataEntry;
        case BatteryCapacity::Id:
            return BatteryCapacity::kMetadataEntry;
        case VehicleID::Id:
            return VehicleID::kMetadataEntry;
        case SessionID::Id:
            return SessionID::kMetadataEntry;
        case SessionDuration::Id:
            return SessionDuration::kMetadataEntry;
        case SessionEnergyCharged::Id:
            return SessionEnergyCharged::kMetadataEntry;
        case SessionEnergyDischarged::Id:
            return SessionEnergyDischarged::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::EnergyEvse::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::EnergyEvse::Commands;
        switch (commandId)
        {
        case Disable::Id:
            return Disable::kMetadataEntry;
        case EnableCharging::Id:
            return EnableCharging::kMetadataEntry;
        case EnableDischarging::Id:
            return EnableDischarging::kMetadataEntry;
        case StartDiagnostics::Id:
            return StartDiagnostics::kMetadataEntry;
        case SetTargets::Id:
            return SetTargets::kMetadataEntry;
        case GetTargets::Id:
            return GetTargets::kMetadataEntry;
        case ClearTargets::Id:
            return ClearTargets::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
