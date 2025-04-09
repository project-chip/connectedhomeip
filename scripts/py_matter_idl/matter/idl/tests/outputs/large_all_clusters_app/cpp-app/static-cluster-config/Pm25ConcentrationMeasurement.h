// DO NOT EDIT - Generated file
//
// Application configuration for Pm25ConcentrationMeasurement based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Pm25ConcentrationMeasurement {
namespace StaticApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<ClusterConfiguration<FeatureBitmapType>, 1> kClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace Pm25ConcentrationMeasurement
} // namespace Clusters
} // namespace app
} // namespace namespace chip

