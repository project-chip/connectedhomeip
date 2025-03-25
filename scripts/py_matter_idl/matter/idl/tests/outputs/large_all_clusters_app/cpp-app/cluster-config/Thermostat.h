// DO NOT EDIT - Generated file
//
// Application configuration for Thermostat
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {
namespace ApplicationConfig {

using FeatureBitmapType = BitFlags<Feature>;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = FeatureBitmapType {
            Feature::kHeating, // feature bit 0x1
            Feature::kCooling, // feature bit 0x2
            Feature::kAutomode // feature bit 0x20
        },
        .clusterRevision = 5,
    },
} };

} // namespace ApplicationConfig
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace namespace chip

