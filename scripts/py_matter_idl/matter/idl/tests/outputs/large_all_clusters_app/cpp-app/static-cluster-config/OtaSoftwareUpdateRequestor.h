// DO NOT EDIT - Generated file
//
// Application configuration for OtaSoftwareUpdateRequestor based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace OtaSoftwareUpdateRequestor {
namespace StaticApplicationConfig {

using FeatureBitmapType = NoFeatureFlagsDefined;

inline constexpr std::array<ClusterConfiguration<FeatureBitmapType>, 1> kClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace OtaSoftwareUpdateRequestor
} // namespace Clusters
} // namespace app
} // namespace namespace chip

