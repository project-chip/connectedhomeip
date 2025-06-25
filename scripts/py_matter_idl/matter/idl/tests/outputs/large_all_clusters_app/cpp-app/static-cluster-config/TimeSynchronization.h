// DO NOT EDIT - Generated file
//
// Application configuration for TimeSynchronization based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/TimeSynchronization/AttributeIds.h>
#include <clusters/TimeSynchronization/CommandIds.h>
#include <clusters/TimeSynchronization/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace TimeSynchronization {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::DefaultNTP::Id,
    Attributes::DSTOffset::Id,
    Attributes::DSTOffsetListMaxSize::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::Granularity::Id,
    Attributes::LocalTime::Id,
    Attributes::SupportsDNSResolve::Id,
    Attributes::TimeSource::Id,
    Attributes::TimeZone::Id,
    Attributes::TimeZoneDatabase::Id,
    Attributes::TimeZoneListMaxSize::Id,
    Attributes::TrustedTimeSource::Id,
    Attributes::UTCTime::Id,
};

inline constexpr CommandId kEndpoint0EnabledCommands[] = {
    Commands::SetDefaultNTP::Id,
    Commands::SetDSTOffset::Id,
    Commands::SetTimeZone::Id,
    Commands::SetTimeZoneResponse::Id,
    Commands::SetTrustedTimeSource::Id,
    Commands::SetUTCTime::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kTimeZone, // feature bit 0x1
            FeatureBitmapType::kNTPClient, // feature bit 0x2
            FeatureBitmapType::kTimeSyncClient// feature bit 0x8
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint0EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint0EnabledCommands),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::DSTOffset::Id:
    case Attributes::DSTOffsetListMaxSize::Id:
    case Attributes::UTCTime::Id:
    case Attributes::AcceptedCommandList::Id:
    case Attributes::AttributeList::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::DefaultNTP::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::Granularity::Id:
    case Attributes::LocalTime::Id:
    case Attributes::SupportsDNSResolve::Id:
    case Attributes::TimeSource::Id:
    case Attributes::TimeZone::Id:
    case Attributes::TimeZoneDatabase::Id:
    case Attributes::TimeZoneListMaxSize::Id:
    case Attributes::TrustedTimeSource::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::SetDSTOffset::Id:
    case Commands::SetDefaultNTP::Id:
    case Commands::SetTimeZone::Id:
    case Commands::SetTimeZoneResponse::Id:
    case Commands::SetTrustedTimeSource::Id:
    case Commands::SetUTCTime::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace chip

