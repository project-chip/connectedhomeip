// DO NOT EDIT - Generated file
//
// Application configuration for UnitTesting
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace UnitTesting {
namespace ApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = 0x0,
        .clusterRevision = 1,
    },
} };

} // namespace ApplicationConfig
} // namespace UnitTesting
} // namespace Clusters
} // namespace app
} // namespace namespace chip

