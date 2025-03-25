// DO NOT EDIT - Generated file
//
// Application configuration for FanControl
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace FanControl {
namespace ApplicationConfig {

using FeatureBitmapType = BitFlags<Feature>;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = FeatureBitmapType {
            Feature::kMultiSpeed, // feature bit 0x1
            Feature::kAuto, // feature bit 0x2
            Feature::kRocking, // feature bit 0x4
            Feature::kWind // feature bit 0x8
        },
        .clusterRevision = 2,
    },
} };

} // namespace ApplicationConfig
} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace namespace chip

