// DO NOT EDIT - Generated file
//
// Application configuration for PumpConfigurationAndControl based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/PumpConfigurationAndControl/AttributeIds.h>
#include <clusters/PumpConfigurationAndControl/CommandIds.h>
#include <clusters/PumpConfigurationAndControl/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace PumpConfigurationAndControl {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::Capacity::Id,
    Attributes::ClusterRevision::Id,
    Attributes::ControlMode::Id,
    Attributes::EffectiveControlMode::Id,
    Attributes::EffectiveOperationMode::Id,
    Attributes::FeatureMap::Id,
    Attributes::LifetimeEnergyConsumed::Id,
    Attributes::LifetimeRunningHours::Id,
    Attributes::MaxCompPressure::Id,
    Attributes::MaxConstFlow::Id,
    Attributes::MaxConstPressure::Id,
    Attributes::MaxConstSpeed::Id,
    Attributes::MaxConstTemp::Id,
    Attributes::MaxFlow::Id,
    Attributes::MaxPressure::Id,
    Attributes::MaxSpeed::Id,
    Attributes::MinCompPressure::Id,
    Attributes::MinConstFlow::Id,
    Attributes::MinConstPressure::Id,
    Attributes::MinConstSpeed::Id,
    Attributes::MinConstTemp::Id,
    Attributes::OperationMode::Id,
    Attributes::Power::Id,
    Attributes::PumpStatus::Id,
    Attributes::Speed::Id,
};
} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kConstantPressure, // feature bit 0x1
            FeatureBitmapType::kCompensatedPressure, // feature bit 0x2
            FeatureBitmapType::kConstantFlow, // feature bit 0x4
            FeatureBitmapType::kConstantSpeed, // feature bit 0x8
            FeatureBitmapType::kConstantTemperature// feature bit 0x10
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::Capacity::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::ControlMode::Id:
    case Attributes::EffectiveControlMode::Id:
    case Attributes::EffectiveOperationMode::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::LifetimeEnergyConsumed::Id:
    case Attributes::LifetimeRunningHours::Id:
    case Attributes::MaxCompPressure::Id:
    case Attributes::MaxConstFlow::Id:
    case Attributes::MaxConstPressure::Id:
    case Attributes::MaxConstSpeed::Id:
    case Attributes::MaxConstTemp::Id:
    case Attributes::MaxFlow::Id:
    case Attributes::MaxPressure::Id:
    case Attributes::MaxSpeed::Id:
    case Attributes::MinCompPressure::Id:
    case Attributes::MinConstFlow::Id:
    case Attributes::MinConstPressure::Id:
    case Attributes::MinConstSpeed::Id:
    case Attributes::MinConstTemp::Id:
    case Attributes::OperationMode::Id:
    case Attributes::Power::Id:
    case Attributes::PumpStatus::Id:
    case Attributes::Speed::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace PumpConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip

