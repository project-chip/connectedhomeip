// DO NOT EDIT - Generated file
//
// Application configuration for ElectricalPowerMeasurement based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/ElectricalPowerMeasurement/AttributeIds.h>
#include <clusters/ElectricalPowerMeasurement/CommandIds.h>
#include <clusters/ElectricalPowerMeasurement/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalPowerMeasurement {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::Accuracy::Id,
    Attributes::ActiveCurrent::Id,
    Attributes::ActivePower::Id,
    Attributes::ApparentCurrent::Id,
    Attributes::ApparentPower::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
    Attributes::Frequency::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::HarmonicCurrents::Id,
    Attributes::HarmonicPhases::Id,
    Attributes::NeutralCurrent::Id,
    Attributes::NumberOfMeasurementTypes::Id,
    Attributes::PowerFactor::Id,
    Attributes::PowerMode::Id,
    Attributes::Ranges::Id,
    Attributes::ReactiveCurrent::Id,
    Attributes::ReactivePower::Id,
    Attributes::RMSCurrent::Id,
    Attributes::RMSPower::Id,
    Attributes::RMSVoltage::Id,
    Attributes::Voltage::Id,
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
    case Attributes::RMSCurrent::Id:
    case Attributes::RMSPower::Id:
    case Attributes::RMSVoltage::Id:
    case Attributes::AcceptedCommandList::Id:
    case Attributes::Accuracy::Id:
    case Attributes::ActiveCurrent::Id:
    case Attributes::ActivePower::Id:
    case Attributes::ApparentCurrent::Id:
    case Attributes::ApparentPower::Id:
    case Attributes::AttributeList::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::Frequency::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::HarmonicCurrents::Id:
    case Attributes::HarmonicPhases::Id:
    case Attributes::NeutralCurrent::Id:
    case Attributes::NumberOfMeasurementTypes::Id:
    case Attributes::PowerFactor::Id:
    case Attributes::PowerMode::Id:
    case Attributes::Ranges::Id:
    case Attributes::ReactiveCurrent::Id:
    case Attributes::ReactivePower::Id:
    case Attributes::Voltage::Id:
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
} // namespace ElectricalPowerMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip

