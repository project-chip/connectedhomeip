// DO NOT EDIT - Generated file
//
// Application configuration for BallastConfiguration based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/BallastConfiguration/AttributeIds.h>
#include <clusters/BallastConfiguration/CommandIds.h>
#include <clusters/BallastConfiguration/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace BallastConfiguration {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::BallastFactorAdjustment::Id,
    Attributes::BallastStatus::Id,
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::IntrinsicBallastFactor::Id,
    Attributes::LampAlarmMode::Id,
    Attributes::LampBurnHours::Id,
    Attributes::LampBurnHoursTripPoint::Id,
    Attributes::LampManufacturer::Id,
    Attributes::LampQuantity::Id,
    Attributes::LampRatedHours::Id,
    Attributes::LampType::Id,
    Attributes::MaxLevel::Id,
    Attributes::MinLevel::Id,
    Attributes::PhysicalMaxLevel::Id,
    Attributes::PhysicalMinLevel::Id,
};
} // namespace detail

using FeatureBitmapType = Clusters::StaticApplicationConfig::NoFeatureFlagsDefined;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::AcceptedCommandList::Id:
    case Attributes::AttributeList::Id:
    case Attributes::BallastFactorAdjustment::Id:
    case Attributes::BallastStatus::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::IntrinsicBallastFactor::Id:
    case Attributes::LampAlarmMode::Id:
    case Attributes::LampBurnHours::Id:
    case Attributes::LampBurnHoursTripPoint::Id:
    case Attributes::LampManufacturer::Id:
    case Attributes::LampQuantity::Id:
    case Attributes::LampRatedHours::Id:
    case Attributes::LampType::Id:
    case Attributes::MaxLevel::Id:
    case Attributes::MinLevel::Id:
    case Attributes::PhysicalMaxLevel::Id:
    case Attributes::PhysicalMinLevel::Id:
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
} // namespace BallastConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip

