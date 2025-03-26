// DO NOT EDIT - Generated file
//
// Application configuration for LevelControl
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace LevelControl {
namespace ApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<FeatureBitmapType, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kOnOff, // feature bit 0x1
            FeatureBitmapType::kLighting// feature bit 0x2
        },
        .clusterRevision = 5,
    },
} };

} // namespace ApplicationConfig
} // namespace LevelControl
} // namespace Clusters
} // namespace app
} // namespace namespace chip

