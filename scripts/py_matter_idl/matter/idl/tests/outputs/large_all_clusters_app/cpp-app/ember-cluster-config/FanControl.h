// DO NOT EDIT - Generated file
//
// Application configuration for FanControl
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace FanControl {
namespace ApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<FeatureBitmapType, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kMultiSpeed, // feature bit 0x1
            FeatureBitmapType::kAuto, // feature bit 0x2
            FeatureBitmapType::kRocking, // feature bit 0x4
            FeatureBitmapType::kWind// feature bit 0x8
        },
        .clusterRevision = 2,
    },
} };

} // namespace ApplicationConfig
} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace namespace chip

