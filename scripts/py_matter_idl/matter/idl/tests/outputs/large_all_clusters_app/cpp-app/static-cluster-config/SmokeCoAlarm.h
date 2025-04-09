// DO NOT EDIT - Generated file
//
// Application configuration for SmokeCoAlarm based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace SmokeCoAlarm {
namespace StaticApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<ClusterConfiguration<FeatureBitmapType>, 1> kClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kSmokeAlarm, // feature bit 0x1
            FeatureBitmapType::kCOAlarm// feature bit 0x2
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace SmokeCoAlarm
} // namespace Clusters
} // namespace app
} // namespace namespace chip

