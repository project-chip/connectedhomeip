// DO NOT EDIT - Generated file
//
// Application configuration for NetworkCommissioning based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {
namespace EmberApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<ClusteConfiguration<FeatureBitmapType>, 2> kFixedEndpoints = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kThreadNetworkInterface// feature bit 0x2
        },
        .clusterRevision = 1,
    },
    {
        .endpointNumber = 65534,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 1,
    },
} };

} // namespace EmberApplicationConfig
} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace namespace chip

