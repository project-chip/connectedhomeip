// DO NOT EDIT - Generated file
//
// Application configuration for OccupancySensing
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace OccupancySensing {
namespace ApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<FeatureBitmapType, 2> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 3,
    },
    {
        .endpointNumber = 2,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 3,
    },
} };

} // namespace ApplicationConfig
} // namespace OccupancySensing
} // namespace Clusters
} // namespace app
} // namespace namespace chip

