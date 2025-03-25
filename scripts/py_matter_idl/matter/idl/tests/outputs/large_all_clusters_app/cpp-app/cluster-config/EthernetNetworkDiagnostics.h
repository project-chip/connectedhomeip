// DO NOT EDIT - Generated file
//
// Application configuration for EthernetNetworkDiagnostics
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace EthernetNetworkDiagnostics {
namespace ApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 0,
        .featureMap = 0x3,
        .clusterRevision = 1,
    },
} };

} // namespace ApplicationConfig
} // namespace EthernetNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace namespace chip

