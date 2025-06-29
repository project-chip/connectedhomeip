// DO NOT EDIT - Generated file
//
// Application configuration for CameraAvStreamManagement based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/CameraAvStreamManagement/AttributeIds.h>
#include <clusters/CameraAvStreamManagement/CommandIds.h>
#include <clusters/CameraAvStreamManagement/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvStreamManagement {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AllocatedSnapshotStreams::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::MaxContentBufferSize::Id,
    Attributes::MaxNetworkBandwidth::Id,
    Attributes::NightVision::Id,
    Attributes::NightVisionIllum::Id,
    Attributes::SupportedSnapshotParams::Id,
    Attributes::SupportedStreamUsages::Id,
};

inline constexpr CommandId kEndpoint1EnabledCommands[] = {
    Commands::CaptureSnapshot::Id,
    Commands::CaptureSnapshotResponse::Id,
    Commands::SetStreamPriorities::Id,
    Commands::SnapshotStreamAllocate::Id,
    Commands::SnapshotStreamAllocateResponse::Id,
    Commands::SnapshotStreamDeallocate::Id,
};

} // namespace detail

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint1EnabledCommands),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::AcceptedCommandList::Id:
    case Attributes::AllocatedSnapshotStreams::Id:
    case Attributes::AttributeList::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::MaxContentBufferSize::Id:
    case Attributes::MaxNetworkBandwidth::Id:
    case Attributes::NightVision::Id:
    case Attributes::NightVisionIllum::Id:
    case Attributes::SupportedSnapshotParams::Id:
    case Attributes::SupportedStreamUsages::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::CaptureSnapshot::Id:
    case Commands::CaptureSnapshotResponse::Id:
    case Commands::SetStreamPriorities::Id:
    case Commands::SnapshotStreamAllocate::Id:
    case Commands::SnapshotStreamAllocateResponse::Id:
    case Commands::SnapshotStreamDeallocate::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip

