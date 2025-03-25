// DO NOT EDIT - Generated file
//
// Application configuration for PowerSource
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerSource {
namespace ApplicationConfig {

using FeatureBitmapType = BitFlags<Feature>;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 3> kFixedEndpoints = { {
    {
        .endpointNumber = 0,
        .featureMap = FeatureBitmapType {
            Feature::kBattery // feature bit 0x2
        },
        .clusterRevision = 1,
    },
    {
        .endpointNumber = 1,
        .featureMap = FeatureBitmapType {
            Feature::kBattery // feature bit 0x2
        },
        .clusterRevision = 1,
    },
    {
        .endpointNumber = 2,
        .featureMap = FeatureBitmapType {
            Feature::kBattery // feature bit 0x2
        },
        .clusterRevision = 1,
    },
} };

} // namespace ApplicationConfig
} // namespace PowerSource
} // namespace Clusters
} // namespace app
} // namespace namespace chip

