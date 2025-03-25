// DO NOT EDIT - Generated file
//
// Application configuration for TemperatureMeasurement
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace TemperatureMeasurement {
namespace ApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = 0x0,
        .clusterRevision = 4,
    },
} };

} // namespace ApplicationConfig
} // namespace TemperatureMeasurement
} // namespace Clusters
} // namespace app
} // namespace namespace chip

