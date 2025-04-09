// DO NOT EDIT - Generated file
//
// Application configuration for TimeSynchronization based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace TimeSynchronization {
namespace StaticApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kTimeZone, // feature bit 0x1
            FeatureBitmapType::kNTPClient, // feature bit 0x2
            FeatureBitmapType::kTimeSyncClient// feature bit 0x8
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace namespace chip

