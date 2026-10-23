// DO NOT EDIT - Generated file
//
// Application configuration for LevelControl based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/LevelControl/AttributeIds.h>
#include <clusters/LevelControl/CommandIds.h>
#include <clusters/LevelControl/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace LevelControl {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::CurrentLevel::Id,
    Attributes::DefaultMoveRate::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::MaxLevel::Id,
    Attributes::MinLevel::Id,
    Attributes::OffTransitionTime::Id,
    Attributes::OnLevel::Id,
    Attributes::OnOffTransitionTime::Id,
    Attributes::OnTransitionTime::Id,
    Attributes::Options::Id,
    Attributes::RemainingTime::Id,
    Attributes::StartUpCurrentLevel::Id,
};

inline constexpr CommandId kEndpoint1EnabledCommands[] = {
    Commands::Move::Id,
    Commands::MoveToLevel::Id,
    Commands::MoveToLevelWithOnOff::Id,
    Commands::MoveWithOnOff::Id,
    Commands::Step::Id,
    Commands::StepWithOnOff::Id,
    Commands::Stop::Id,
    Commands::StopWithOnOff::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kOnOff, // feature bit 0x1
            FeatureBitmapType::kLighting// feature bit 0x2
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint1EnabledCommands),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::AcceptedCommandList::Id:
    case Attributes::AttributeList::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::CurrentLevel::Id:
    case Attributes::DefaultMoveRate::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::MaxLevel::Id:
    case Attributes::MinLevel::Id:
    case Attributes::OffTransitionTime::Id:
    case Attributes::OnLevel::Id:
    case Attributes::OnOffTransitionTime::Id:
    case Attributes::OnTransitionTime::Id:
    case Attributes::Options::Id:
    case Attributes::RemainingTime::Id:
    case Attributes::StartUpCurrentLevel::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::Move::Id:
    case Commands::MoveToLevel::Id:
    case Commands::MoveToLevelWithOnOff::Id:
    case Commands::MoveWithOnOff::Id:
    case Commands::Step::Id:
    case Commands::StepWithOnOff::Id:
    case Commands::Stop::Id:
    case Commands::StopWithOnOff::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace LevelControl
} // namespace Clusters
} // namespace app
} // namespace chip

