// DO NOT EDIT - Generated file
//
// Application configuration for ContentLauncher
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace ContentLauncher {
namespace ApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<FeatureBitmapType, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 1,
    },
} };

} // namespace ApplicationConfig
} // namespace ContentLauncher
} // namespace Clusters
} // namespace app
} // namespace namespace chip

