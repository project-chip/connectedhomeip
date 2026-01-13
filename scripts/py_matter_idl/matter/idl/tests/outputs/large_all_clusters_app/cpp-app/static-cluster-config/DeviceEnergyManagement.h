// DO NOT EDIT - Generated file
//
// Application configuration for DeviceEnergyManagement based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/DeviceEnergyManagement/AttributeIds.h>
#include <clusters/DeviceEnergyManagement/CommandIds.h>
#include <clusters/DeviceEnergyManagement/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AbsMaxPower::Id,
    Attributes::AbsMinPower::Id,
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::ESACanGenerate::Id,
    Attributes::ESAState::Id,
    Attributes::ESAType::Id,
    Attributes::FeatureMap::Id,
    Attributes::Forecast::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::OptOutState::Id,
    Attributes::PowerAdjustmentCapability::Id,
};

inline constexpr CommandId kEndpoint1EnabledCommands[] = {
    Commands::CancelPowerAdjustRequest::Id,
    Commands::CancelRequest::Id,
    Commands::ModifyForecastRequest::Id,
    Commands::PauseRequest::Id,
    Commands::PowerAdjustRequest::Id,
    Commands::RequestConstraintBasedForecast::Id,
    Commands::ResumeRequest::Id,
    Commands::StartTimeAdjustRequest::Id,
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
    case Attributes::ESACanGenerate::Id:
    case Attributes::ESAState::Id:
    case Attributes::ESAType::Id:
    case Attributes::AbsMaxPower::Id:
    case Attributes::AbsMinPower::Id:
    case Attributes::AcceptedCommandList::Id:
    case Attributes::AttributeList::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::Forecast::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::OptOutState::Id:
    case Attributes::PowerAdjustmentCapability::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::CancelPowerAdjustRequest::Id:
    case Commands::CancelRequest::Id:
    case Commands::ModifyForecastRequest::Id:
    case Commands::PauseRequest::Id:
    case Commands::PowerAdjustRequest::Id:
    case Commands::RequestConstraintBasedForecast::Id:
    case Commands::ResumeRequest::Id:
    case Commands::StartTimeAdjustRequest::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip

