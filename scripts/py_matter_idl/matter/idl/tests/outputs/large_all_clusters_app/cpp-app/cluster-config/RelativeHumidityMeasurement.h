// DO NOT EDIT - Generated file
//
// Application configuration for RelativeHumidityMeasurement
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace RelativeHumidityMeasurement {
namespace ApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 2> kFixedEndpoints = { {
    {
        .endpointNumber = 0,
        .featureMap = 0x0,
        .clusterRevision = 3,
    },
    {
        .endpointNumber = 1,
        .featureMap = 0x0,
        .clusterRevision = 3,
    },
} };

} // namespace ApplicationConfig
} // namespace RelativeHumidityMeasurement
} // namespace Clusters
} // namespace app
} // namespace namespace chip

