// DO NOT EDIT - Generated file
//
// Application configuration for TimeFormatLocalization based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace TimeFormatLocalization {
namespace StaticApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<ClusterConfiguration<FeatureBitmapType>, 1> kClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kCalendarFormat// feature bit 0x1
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace TimeFormatLocalization
} // namespace Clusters
} // namespace app
} // namespace namespace chip

