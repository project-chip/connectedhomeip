// DO NOT EDIT - Generated file
//
// Application configuration for TemperatureControl based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace TemperatureControl {
namespace StaticApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kTemperatureLevel// feature bit 0x2
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace TemperatureControl
} // namespace Clusters
} // namespace app
} // namespace chip

