// DO NOT EDIT - Generated file
//
// Application configuration for Pm10ConcentrationMeasurement based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/Pm10ConcentrationMeasurement/AttributeIds.h>
#include <clusters/Pm10ConcentrationMeasurement/CommandIds.h>
#include <clusters/Pm10ConcentrationMeasurement/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Pm10ConcentrationMeasurement {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::AverageMeasuredValue::Id,
    Attributes::AverageMeasuredValueWindow::Id,
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::LevelValue::Id,
    Attributes::MaxMeasuredValue::Id,
    Attributes::MeasuredValue::Id,
    Attributes::MeasurementMedium::Id,
    Attributes::MeasurementUnit::Id,
    Attributes::MinMeasuredValue::Id,
    Attributes::PeakMeasuredValue::Id,
    Attributes::PeakMeasuredValueWindow::Id,
    Attributes::Uncertainty::Id,
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
    case Attributes::AttributeList::Id:
    case Attributes::AverageMeasuredValue::Id:
    case Attributes::AverageMeasuredValueWindow::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::LevelValue::Id:
    case Attributes::MaxMeasuredValue::Id:
    case Attributes::MeasuredValue::Id:
    case Attributes::MeasurementMedium::Id:
    case Attributes::MeasurementUnit::Id:
    case Attributes::MinMeasuredValue::Id:
    case Attributes::PeakMeasuredValue::Id:
    case Attributes::PeakMeasuredValueWindow::Id:
    case Attributes::Uncertainty::Id:
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
} // namespace Pm10ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip

