// DO NOT EDIT - Generated file
//
// Application configuration for WindowCovering based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {
namespace EmberApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<EmberApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kEmberClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kLift, // feature bit 0x1
            FeatureBitmapType::kTilt, // feature bit 0x2
            FeatureBitmapType::kPositionAwareLift, // feature bit 0x4
            FeatureBitmapType::kPositionAwareTilt// feature bit 0x10
        },
    },
} };

} // namespace EmberApplicationConfig
} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace namespace chip

