// DO NOT EDIT - Generated file
//
// Application configuration for OccupancySensing based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace OccupancySensing {
namespace EmberApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<ClusteConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 3,
    },
} };

} // namespace EmberApplicationConfig
} // namespace OccupancySensing
} // namespace Clusters
} // namespace app
} // namespace namespace chip

