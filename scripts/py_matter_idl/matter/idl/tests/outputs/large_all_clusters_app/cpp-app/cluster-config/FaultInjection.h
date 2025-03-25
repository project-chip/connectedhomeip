// DO NOT EDIT - Generated file
//
// Application configuration for FaultInjection
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace FaultInjection {
namespace ApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 0,
        .featureMap = 0x0,
        .clusterRevision = 1,
    },
} };

} // namespace ApplicationConfig
} // namespace FaultInjection
} // namespace Clusters
} // namespace app
} // namespace namespace chip

