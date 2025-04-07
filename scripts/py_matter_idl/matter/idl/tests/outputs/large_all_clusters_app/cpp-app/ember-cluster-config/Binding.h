// DO NOT EDIT - Generated file
//
// Application configuration for Binding based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Binding {
namespace EmberApplicationConfig {

using FeatureBitmapType = NoFeatureFlagsDefined;

inline constexpr std::array<EmberApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 2> kEmberClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
    },
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
    },
} };

} // namespace EmberApplicationConfig
} // namespace Binding
} // namespace Clusters
} // namespace app
} // namespace namespace chip

