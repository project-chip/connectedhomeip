// DO NOT EDIT - Generated file
//
// Application configuration for OnOff based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace OnOff {
namespace EmberApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<ClusteConfiguration<FeatureBitmapType>, 2> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kLighting// feature bit 0x1
        },
        .clusterRevision = 4,
    },
    {
        .endpointNumber = 2,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 4,
    },
} };

} // namespace EmberApplicationConfig
} // namespace OnOff
} // namespace Clusters
} // namespace app
} // namespace namespace chip

