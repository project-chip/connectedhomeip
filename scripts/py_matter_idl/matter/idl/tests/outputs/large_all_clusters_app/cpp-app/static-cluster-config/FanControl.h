// DO NOT EDIT - Generated file
//
// Application configuration for FanControl based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace FanControl {
namespace StaticApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kMultiSpeed, // feature bit 0x1
            FeatureBitmapType::kAuto, // feature bit 0x2
            FeatureBitmapType::kRocking, // feature bit 0x4
            FeatureBitmapType::kWind, // feature bit 0x8
            FeatureBitmapType::kStep, // feature bit 0x10
            FeatureBitmapType::kAirflowDirection// feature bit 0x20
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace chip

