// DO NOT EDIT - Generated file
//
// Application configuration for DishwasherAlarm based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace DishwasherAlarm {
namespace StaticApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<ClusterConfiguration<FeatureBitmapType>, 1> kClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kReset// feature bit 0x1
        },
    },
} };

} // namespace EmberApplicationConfig
} // namespace DishwasherAlarm
} // namespace Clusters
} // namespace app
} // namespace namespace chip

