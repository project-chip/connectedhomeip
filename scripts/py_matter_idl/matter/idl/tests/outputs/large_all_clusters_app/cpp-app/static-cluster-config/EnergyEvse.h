// DO NOT EDIT - Generated file
//
// Application configuration for EnergyEvse based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/EnergyEvse/AttributeIds.h>
#include <clusters/EnergyEvse/CommandIds.h>
#include <clusters/EnergyEvse/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::ApproximateEVEfficiency::Id,
    Attributes::AttributeList::Id,
    Attributes::ChargingEnabledUntil::Id,
    Attributes::CircuitCapacity::Id,
    Attributes::ClusterRevision::Id,
    Attributes::FaultState::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::MaximumChargeCurrent::Id,
    Attributes::MinimumChargeCurrent::Id,
    Attributes::NextChargeRequiredEnergy::Id,
    Attributes::NextChargeStartTime::Id,
    Attributes::NextChargeTargetSoC::Id,
    Attributes::NextChargeTargetTime::Id,
    Attributes::RandomizationDelayWindow::Id,
    Attributes::SessionDuration::Id,
    Attributes::SessionEnergyCharged::Id,
    Attributes::SessionID::Id,
    Attributes::State::Id,
    Attributes::SupplyState::Id,
    Attributes::UserMaximumChargeCurrent::Id,
};

inline constexpr CommandId kEndpoint1EnabledCommands[] = {
    Commands::ClearTargets::Id,
    Commands::Disable::Id,
    Commands::EnableCharging::Id,
    Commands::EnableDischarging::Id,
    Commands::GetTargets::Id,
    Commands::GetTargetsResponse::Id,
    Commands::SetTargets::Id,
    Commands::StartDiagnostics::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint1EnabledCommands),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::AcceptedCommandList::Id:
    case Attributes::ApproximateEVEfficiency::Id:
    case Attributes::AttributeList::Id:
    case Attributes::ChargingEnabledUntil::Id:
    case Attributes::CircuitCapacity::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::FaultState::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::MaximumChargeCurrent::Id:
    case Attributes::MinimumChargeCurrent::Id:
    case Attributes::NextChargeRequiredEnergy::Id:
    case Attributes::NextChargeStartTime::Id:
    case Attributes::NextChargeTargetSoC::Id:
    case Attributes::NextChargeTargetTime::Id:
    case Attributes::RandomizationDelayWindow::Id:
    case Attributes::SessionDuration::Id:
    case Attributes::SessionEnergyCharged::Id:
    case Attributes::SessionID::Id:
    case Attributes::State::Id:
    case Attributes::SupplyState::Id:
    case Attributes::UserMaximumChargeCurrent::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::ClearTargets::Id:
    case Commands::Disable::Id:
    case Commands::EnableCharging::Id:
    case Commands::EnableDischarging::Id:
    case Commands::GetTargets::Id:
    case Commands::GetTargetsResponse::Id:
    case Commands::SetTargets::Id:
    case Commands::StartDiagnostics::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip

