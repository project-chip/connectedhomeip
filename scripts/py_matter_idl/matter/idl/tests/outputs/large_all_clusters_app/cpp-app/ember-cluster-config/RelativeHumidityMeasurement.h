// DO NOT EDIT - Generated file
//
// Application configuration for RelativeHumidityMeasurement based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace RelativeHumidityMeasurement {
namespace EmberApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<ClusteConfiguration<FeatureBitmapType>, 2> kFixedEndpoints = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 3,
    },
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 3,
    },
} };

} // namespace EmberApplicationConfig
} // namespace RelativeHumidityMeasurement
} // namespace Clusters
} // namespace app
} // namespace namespace chip

