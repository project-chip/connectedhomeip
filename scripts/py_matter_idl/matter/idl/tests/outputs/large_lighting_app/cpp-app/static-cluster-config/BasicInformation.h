// DO NOT EDIT - Generated file
//
// Application configuration for BasicInformation based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/BasicInformation/AttributeIds.h>
#include <clusters/BasicInformation/CommandIds.h>
#include <clusters/BasicInformation/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace BasicInformation {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::CapabilityMinima::Id,
    Attributes::ClusterRevision::Id,
    Attributes::DataModelRevision::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::HardwareVersion::Id,
    Attributes::HardwareVersionString::Id,
    Attributes::LocalConfigDisabled::Id,
    Attributes::Location::Id,
    Attributes::ManufacturingDate::Id,
    Attributes::MaxPathsPerInvoke::Id,
    Attributes::NodeLabel::Id,
    Attributes::PartNumber::Id,
    Attributes::ProductID::Id,
    Attributes::ProductLabel::Id,
    Attributes::ProductName::Id,
    Attributes::ProductURL::Id,
    Attributes::SerialNumber::Id,
    Attributes::SoftwareVersion::Id,
    Attributes::SoftwareVersionString::Id,
    Attributes::SpecificationVersion::Id,
    Attributes::UniqueID::Id,
    Attributes::VendorID::Id,
    Attributes::VendorName::Id,
};
} // namespace detail

using FeatureBitmapType = Clusters::StaticApplicationConfig::NoFeatureFlagsDefined;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint0EnabledAttributes),
        .enabledCommands = Span<const CommandId>(),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::AcceptedCommandList::Id:
    case Attributes::AttributeList::Id:
    case Attributes::CapabilityMinima::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::DataModelRevision::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::HardwareVersion::Id:
    case Attributes::HardwareVersionString::Id:
    case Attributes::LocalConfigDisabled::Id:
    case Attributes::Location::Id:
    case Attributes::ManufacturingDate::Id:
    case Attributes::MaxPathsPerInvoke::Id:
    case Attributes::NodeLabel::Id:
    case Attributes::PartNumber::Id:
    case Attributes::ProductID::Id:
    case Attributes::ProductLabel::Id:
    case Attributes::ProductName::Id:
    case Attributes::ProductURL::Id:
    case Attributes::SerialNumber::Id:
    case Attributes::SoftwareVersion::Id:
    case Attributes::SoftwareVersionString::Id:
    case Attributes::SpecificationVersion::Id:
    case Attributes::UniqueID::Id:
    case Attributes::VendorID::Id:
    case Attributes::VendorName::Id:
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
} // namespace BasicInformation
} // namespace Clusters
} // namespace app
} // namespace chip

