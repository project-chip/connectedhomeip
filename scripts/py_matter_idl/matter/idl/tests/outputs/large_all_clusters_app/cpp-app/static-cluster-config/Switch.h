// DO NOT EDIT - Generated file
//
// Application configuration for Switch based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/Switch/AttributeIds.h>
#include <clusters/Switch/CommandIds.h>
#include <clusters/Switch/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Switch {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::ClusterRevision::Id,
    Attributes::CurrentPosition::Id,
    Attributes::FeatureMap::Id,
    Attributes::NumberOfPositions::Id,
};
inline constexpr AttributeId kEndpoint3EnabledAttributes[] = {
    Attributes::ClusterRevision::Id,
    Attributes::CurrentPosition::Id,
    Attributes::FeatureMap::Id,
    Attributes::MultiPressMax::Id,
    Attributes::NumberOfPositions::Id,
};
inline constexpr AttributeId kEndpoint4EnabledAttributes[] = {
    Attributes::ClusterRevision::Id,
    Attributes::CurrentPosition::Id,
    Attributes::FeatureMap::Id,
    Attributes::MultiPressMax::Id,
    Attributes::NumberOfPositions::Id,
};
} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 3> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kLatchingSwitch// feature bit 0x1
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(),
    },
    {
        .endpointNumber = 3,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kMomentarySwitch, // feature bit 0x2
            FeatureBitmapType::kMomentarySwitchLongPress, // feature bit 0x8
            FeatureBitmapType::kMomentarySwitchMultiPress, // feature bit 0x10
            FeatureBitmapType::kActionSwitch// feature bit 0x20
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint3EnabledAttributes),
        .enabledCommands = Span<const CommandId>(),
    },
    {
        .endpointNumber = 4,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kMomentarySwitch, // feature bit 0x2
            FeatureBitmapType::kMomentarySwitchRelease, // feature bit 0x4
            FeatureBitmapType::kMomentarySwitchLongPress, // feature bit 0x8
            FeatureBitmapType::kMomentarySwitchMultiPress// feature bit 0x10
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint4EnabledAttributes),
        .enabledCommands = Span<const CommandId>(),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::ClusterRevision::Id:
    case Attributes::CurrentPosition::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::MultiPressMax::Id:
    case Attributes::NumberOfPositions::Id:
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
} // namespace Switch
} // namespace Clusters
} // namespace app
} // namespace chip

