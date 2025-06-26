// DO NOT EDIT - Generated file
//
// Application configuration for Thermostat based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/Thermostat/AttributeIds.h>
#include <clusters/Thermostat/CommandIds.h>
#include <clusters/Thermostat/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AbsMaxCoolSetpointLimit::Id,
    Attributes::AbsMaxHeatSetpointLimit::Id,
    Attributes::AbsMinCoolSetpointLimit::Id,
    Attributes::AbsMinHeatSetpointLimit::Id,
    Attributes::AcceptedCommandList::Id,
    Attributes::ActivePresetHandle::Id,
    Attributes::ActiveScheduleHandle::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::ControlSequenceOfOperation::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::LocalTemperature::Id,
    Attributes::LocalTemperatureCalibration::Id,
    Attributes::MaxCoolSetpointLimit::Id,
    Attributes::MaxHeatSetpointLimit::Id,
    Attributes::MinCoolSetpointLimit::Id,
    Attributes::MinHeatSetpointLimit::Id,
    Attributes::MinSetpointDeadBand::Id,
    Attributes::NumberOfPresets::Id,
    Attributes::NumberOfSchedules::Id,
    Attributes::NumberOfScheduleTransitionPerDay::Id,
    Attributes::OccupiedCoolingSetpoint::Id,
    Attributes::OccupiedHeatingSetpoint::Id,
    Attributes::Presets::Id,
    Attributes::PresetTypes::Id,
    Attributes::Schedules::Id,
    Attributes::ScheduleTypes::Id,
    Attributes::SetpointHoldExpiryTimestamp::Id,
    Attributes::SystemMode::Id,
};

inline constexpr CommandId kEndpoint1EnabledCommands[] = {
    Commands::AtomicRequest::Id,
    Commands::AtomicResponse::Id,
    Commands::SetActivePresetRequest::Id,
    Commands::SetActiveScheduleRequest::Id,
    Commands::SetpointRaiseLower::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kHeating, // feature bit 0x1
            FeatureBitmapType::kCooling, // feature bit 0x2
            FeatureBitmapType::kAutoMode, // feature bit 0x20
            FeatureBitmapType::kPresets// feature bit 0x100
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint1EnabledCommands),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::AbsMaxCoolSetpointLimit::Id:
    case Attributes::AbsMaxHeatSetpointLimit::Id:
    case Attributes::AbsMinCoolSetpointLimit::Id:
    case Attributes::AbsMinHeatSetpointLimit::Id:
    case Attributes::AcceptedCommandList::Id:
    case Attributes::ActivePresetHandle::Id:
    case Attributes::ActiveScheduleHandle::Id:
    case Attributes::AttributeList::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::ControlSequenceOfOperation::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::LocalTemperature::Id:
    case Attributes::LocalTemperatureCalibration::Id:
    case Attributes::MaxCoolSetpointLimit::Id:
    case Attributes::MaxHeatSetpointLimit::Id:
    case Attributes::MinCoolSetpointLimit::Id:
    case Attributes::MinHeatSetpointLimit::Id:
    case Attributes::MinSetpointDeadBand::Id:
    case Attributes::NumberOfPresets::Id:
    case Attributes::NumberOfScheduleTransitionPerDay::Id:
    case Attributes::NumberOfSchedules::Id:
    case Attributes::OccupiedCoolingSetpoint::Id:
    case Attributes::OccupiedHeatingSetpoint::Id:
    case Attributes::PresetTypes::Id:
    case Attributes::Presets::Id:
    case Attributes::ScheduleTypes::Id:
    case Attributes::Schedules::Id:
    case Attributes::SetpointHoldExpiryTimestamp::Id:
    case Attributes::SystemMode::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::AtomicRequest::Id:
    case Commands::AtomicResponse::Id:
    case Commands::SetActivePresetRequest::Id:
    case Commands::SetActiveScheduleRequest::Id:
    case Commands::SetpointRaiseLower::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

