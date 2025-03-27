// DO NOT EDIT - Generated file
//
// Application configuration for ColorControl based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace ColorControl {
namespace EmberApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<EmberApplicationConfig::ClusteConfiguration<FeatureBitmapType>, 1> kEmberClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kHueAndSaturation, // feature bit 0x1
            FeatureBitmapType::kEnhancedHue, // feature bit 0x2
            FeatureBitmapType::kColorLoop, // feature bit 0x4
            FeatureBitmapType::kXy, // feature bit 0x8
            FeatureBitmapType::kColorTemperature// feature bit 0x10
        },
    },
} };

} // namespace EmberApplicationConfig
} // namespace ColorControl
} // namespace Clusters
} // namespace app
} // namespace namespace chip

