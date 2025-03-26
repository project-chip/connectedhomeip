// DO NOT EDIT - Generated file
//
// Application configuration for LevelControl based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace LevelControl {
namespace EmberApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<EmberApplicationConfig::ClusteConfiguration<FeatureBitmapType>, 1> kEmberClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kOnOff, // feature bit 0x1
            FeatureBitmapType::kLighting// feature bit 0x2
        },
        .clusterRevision = 5,
    },
} };

} // namespace EmberApplicationConfig
} // namespace LevelControl
} // namespace Clusters
} // namespace app
} // namespace namespace chip

