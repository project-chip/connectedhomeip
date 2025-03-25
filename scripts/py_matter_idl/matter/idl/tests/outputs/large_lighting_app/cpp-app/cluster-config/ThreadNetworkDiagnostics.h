// DO NOT EDIT - Generated file
//
// Application configuration for ThreadNetworkDiagnostics
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadNetworkDiagnostics {
namespace ApplicationConfig {

using FeatureBitmapType = BitFlags<Feature>;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 0,
        .featureMap = FeatureBitmapType {
            Feature::kPacketCounts, // feature bit 0x1
            Feature::kErrorCounts, // feature bit 0x2
            Feature::kMLECounts, // feature bit 0x4
            Feature::kMACCounts // feature bit 0x8
        },
        .clusterRevision = 1,
    },
} };

} // namespace ApplicationConfig
} // namespace ThreadNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace namespace chip

