// DO NOT EDIT - Generated file
//
// Application configuration for NetworkCommissioning
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {
namespace ApplicationConfig {

using FeatureBitmapType = BitFlags<Feature>;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 0,
        .featureMap = FeatureBitmapType {
            Feature::kThreadNetworkInterface // feature bit 0x2
        },
        .clusterRevision = 1,
    },
} };

} // namespace ApplicationConfig
} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace namespace chip

