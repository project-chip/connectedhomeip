// DO NOT EDIT - Generated file
//
// Application configuration for WindowCovering
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {
namespace ApplicationConfig {

using FeatureBitmapType = BitFlags<Feature>;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = FeatureBitmapType {
            Feature::kLift, // feature bit 0x1
            Feature::kTilt, // feature bit 0x2
            Feature::kPositionAwareLift, // feature bit 0x4
            Feature::kPositionAwareTilt // feature bit 0x10
        },
        .clusterRevision = 5,
    },
} };

} // namespace ApplicationConfig
} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace namespace chip

