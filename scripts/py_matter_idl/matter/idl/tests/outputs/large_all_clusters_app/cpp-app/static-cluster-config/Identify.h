// DO NOT EDIT - Generated file
//
// Application configuration for Identify based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Identify {
namespace StaticApplicationConfig {

using FeatureBitmapType = NoFeatureFlagsDefined;

inline constexpr std::array<ClusterConfiguration<FeatureBitmapType>, 4> kClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
    },
    {
        .endpointNumber = 2,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
    },
    {
        .endpointNumber = 3,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
    },
    {
        .endpointNumber = 4,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace Identify
} // namespace Clusters
} // namespace app
} // namespace namespace chip

