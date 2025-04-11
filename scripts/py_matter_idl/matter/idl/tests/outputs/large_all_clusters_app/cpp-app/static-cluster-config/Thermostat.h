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
namespace StaticApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kHeating, // feature bit 0x1
            FeatureBitmapType::kCooling, // feature bit 0x2
            FeatureBitmapType::kAutoMode, // feature bit 0x20
            FeatureBitmapType::kPresets// feature bit 0x100
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

