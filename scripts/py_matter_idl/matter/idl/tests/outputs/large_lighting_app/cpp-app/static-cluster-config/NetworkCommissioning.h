// DO NOT EDIT - Generated file
//
// Application configuration for NetworkCommissioning based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {
namespace StaticApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kThreadNetworkInterface// feature bit 0x2
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace namespace chip

