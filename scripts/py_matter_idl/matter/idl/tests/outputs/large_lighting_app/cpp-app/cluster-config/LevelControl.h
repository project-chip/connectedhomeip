// DO NOT EDIT - Generated file
//
// Application configuration for LevelControl
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace LevelControl {
namespace ApplicationConfig {

using FeatureBitmapType = BitFlags<Feature>;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = FeatureBitmapType {
            Feature::kOnOff, // feature bit 0x1
            Feature::kLighting // feature bit 0x2
        },
        .clusterRevision = 5,
    },
} };

} // namespace ApplicationConfig
} // namespace LevelControl
} // namespace Clusters
} // namespace app
} // namespace namespace chip

