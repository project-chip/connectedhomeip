// DO NOT EDIT - Generated file
//
// Application configuration for ColorControl
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace ColorControl {
namespace ApplicationConfig {

using FeatureBitmapType = BitFlags<Feature>;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = FeatureBitmapType {
            Feature::kHueAndSaturation, // feature bit 0x1
            Feature::kEnhancedHue, // feature bit 0x2
            Feature::kColorLoop, // feature bit 0x4
            Feature::kXy, // feature bit 0x8
            Feature::kColorTemperature // feature bit 0x10
        },
        .clusterRevision = 5,
    },
} };

} // namespace ApplicationConfig
} // namespace ColorControl
} // namespace Clusters
} // namespace app
} // namespace namespace chip

