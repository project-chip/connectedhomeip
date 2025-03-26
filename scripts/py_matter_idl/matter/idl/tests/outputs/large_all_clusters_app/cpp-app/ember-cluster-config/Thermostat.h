// DO NOT EDIT - Generated file
//
// Application configuration for Thermostat based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {
namespace EmberApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<ClusteConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kHeating, // feature bit 0x1
            FeatureBitmapType::kCooling, // feature bit 0x2
            FeatureBitmapType::kAutomode// feature bit 0x20
        },
        .clusterRevision = 5,
    },
} };

} // namespace EmberApplicationConfig
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace namespace chip

