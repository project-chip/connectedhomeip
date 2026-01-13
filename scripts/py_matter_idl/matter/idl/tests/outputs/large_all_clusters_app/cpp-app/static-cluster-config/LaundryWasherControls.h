// DO NOT EDIT - Generated file
//
// Application configuration for LaundryWasherControls based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/LaundryWasherControls/AttributeIds.h>
#include <clusters/LaundryWasherControls/CommandIds.h>
#include <clusters/LaundryWasherControls/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace LaundryWasherControls {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::NumberOfRinses::Id,
    Attributes::SpinSpeedCurrent::Id,
    Attributes::SpinSpeeds::Id,
    Attributes::SupportedRinses::Id,
};
} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kSpin, // feature bit 0x1
            FeatureBitmapType::kRinse// feature bit 0x2
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
    case Attributes::ClusterRevision::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::NumberOfRinses::Id:
    case Attributes::SpinSpeedCurrent::Id:
    case Attributes::SpinSpeeds::Id:
    case Attributes::SupportedRinses::Id:
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
} // namespace LaundryWasherControls
} // namespace Clusters
} // namespace app
} // namespace chip

