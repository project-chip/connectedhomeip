// DO NOT EDIT - Generated file
//
// Application configuration for ElectricalEnergyMeasurement based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/ElectricalEnergyMeasurement/AttributeIds.h>
#include <clusters/ElectricalEnergyMeasurement/CommandIds.h>
#include <clusters/ElectricalEnergyMeasurement/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::Accuracy::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::CumulativeEnergyExported::Id,
    Attributes::CumulativeEnergyImported::Id,
    Attributes::CumulativeEnergyReset::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::PeriodicEnergyExported::Id,
    Attributes::PeriodicEnergyImported::Id,
};
} // namespace detail

using FeatureBitmapType = Feature;

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
    case Attributes::Accuracy::Id:
    case Attributes::AttributeList::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::CumulativeEnergyExported::Id:
    case Attributes::CumulativeEnergyImported::Id:
    case Attributes::CumulativeEnergyReset::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::PeriodicEnergyExported::Id:
    case Attributes::PeriodicEnergyImported::Id:
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
} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip

