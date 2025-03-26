// DO NOT EDIT - Generated file
//
// Application configuration for PowerSource based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerSource {
namespace ApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<FeatureBitmapType, 3> kFixedEndpoints = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kBattery// feature bit 0x2
        },
        .clusterRevision = 1,
    },
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kBattery// feature bit 0x2
        },
        .clusterRevision = 1,
    },
    {
        .endpointNumber = 2,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kBattery// feature bit 0x2
        },
        .clusterRevision = 1,
    },
} };

} // namespace ApplicationConfig
} // namespace PowerSource
} // namespace Clusters
} // namespace app
} // namespace namespace chip

