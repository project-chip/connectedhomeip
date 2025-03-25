// DO NOT EDIT - Generated file
//
// Application configuration for OnOff
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace OnOff {
namespace ApplicationConfig {

using FeatureBitmapType = BitFlags<Feature>;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 2> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = FeatureBitmapType {
            Feature::kLighting // feature bit 0x1
        },
        .clusterRevision = 4,
    },
    {
        .endpointNumber = 2,
        .featureMap = FeatureBitmapType {
        },
        .clusterRevision = 4,
    },
} };

} // namespace ApplicationConfig
} // namespace OnOff
} // namespace Clusters
} // namespace app
} // namespace namespace chip

