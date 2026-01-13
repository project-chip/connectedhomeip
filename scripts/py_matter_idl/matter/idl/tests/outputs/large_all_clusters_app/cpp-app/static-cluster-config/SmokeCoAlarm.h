// DO NOT EDIT - Generated file
//
// Application configuration for SmokeCoAlarm based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/SmokeCoAlarm/AttributeIds.h>
#include <clusters/SmokeCoAlarm/CommandIds.h>
#include <clusters/SmokeCoAlarm/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace SmokeCoAlarm {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::BatteryAlert::Id,
    Attributes::ClusterRevision::Id,
    Attributes::ContaminationState::Id,
    Attributes::COState::Id,
    Attributes::DeviceMuted::Id,
    Attributes::EndOfServiceAlert::Id,
    Attributes::ExpiryDate::Id,
    Attributes::ExpressedState::Id,
    Attributes::FeatureMap::Id,
    Attributes::HardwareFaultAlert::Id,
    Attributes::InterconnectCOAlarm::Id,
    Attributes::InterconnectSmokeAlarm::Id,
    Attributes::SmokeSensitivityLevel::Id,
    Attributes::SmokeState::Id,
    Attributes::TestInProgress::Id,
};

inline constexpr CommandId kEndpoint1EnabledCommands[] = {
    Commands::SelfTestRequest::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kSmokeAlarm, // feature bit 0x1
            FeatureBitmapType::kCOAlarm// feature bit 0x2
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint1EnabledCommands),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::COState::Id:
    case Attributes::BatteryAlert::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::ContaminationState::Id:
    case Attributes::DeviceMuted::Id:
    case Attributes::EndOfServiceAlert::Id:
    case Attributes::ExpiryDate::Id:
    case Attributes::ExpressedState::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::HardwareFaultAlert::Id:
    case Attributes::InterconnectCOAlarm::Id:
    case Attributes::InterconnectSmokeAlarm::Id:
    case Attributes::SmokeSensitivityLevel::Id:
    case Attributes::SmokeState::Id:
    case Attributes::TestInProgress::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::SelfTestRequest::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace SmokeCoAlarm
} // namespace Clusters
} // namespace app
} // namespace chip

