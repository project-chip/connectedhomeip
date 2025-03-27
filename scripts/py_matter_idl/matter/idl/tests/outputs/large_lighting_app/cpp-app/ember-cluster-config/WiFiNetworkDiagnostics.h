// DO NOT EDIT - Generated file
//
// Application configuration for WiFiNetworkDiagnostics based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace WiFiNetworkDiagnostics {
namespace EmberApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<EmberApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kEmberClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
    },
} };

} // namespace EmberApplicationConfig
} // namespace WiFiNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace namespace chip

