// DO NOT EDIT - Generated file
//
// Application configuration for EthernetNetworkDiagnostics based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace EthernetNetworkDiagnostics {
namespace StaticApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kPacketCounts, // feature bit 0x1
            FeatureBitmapType::kErrorCounts// feature bit 0x2
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace EthernetNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip

