// DO NOT EDIT - Generated file
//
// Application configuration for OccupancySensing based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/OccupancySensing/AttributeIds.h>
#include <clusters/OccupancySensing/CommandIds.h>
#include <clusters/OccupancySensing/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace OccupancySensing {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
    Attributes::HoldTime::Id,
    Attributes::HoldTimeLimits::Id,
    Attributes::Occupancy::Id,
    Attributes::OccupancySensorType::Id,
    Attributes::OccupancySensorTypeBitmap::Id,
    Attributes::PIROccupiedToUnoccupiedDelay::Id,
};
inline constexpr AttributeId kEndpoint2EnabledAttributes[] = {
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
    Attributes::HoldTime::Id,
    Attributes::HoldTimeLimits::Id,
    Attributes::Occupancy::Id,
    Attributes::OccupancySensorType::Id,
    Attributes::OccupancySensorTypeBitmap::Id,
    Attributes::PIROccupiedToUnoccupiedDelay::Id,
};
} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 2> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(),
    },
    {
        .endpointNumber = 2,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint2EnabledAttributes),
        .enabledCommands = Span<const CommandId>(),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::PIROccupiedToUnoccupiedDelay::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::HoldTime::Id:
    case Attributes::HoldTimeLimits::Id:
    case Attributes::Occupancy::Id:
    case Attributes::OccupancySensorType::Id:
    case Attributes::OccupancySensorTypeBitmap::Id:
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
} // namespace OccupancySensing
} // namespace Clusters
} // namespace app
} // namespace chip

