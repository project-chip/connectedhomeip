// DO NOT EDIT - Generated file
//
// Application configuration for DiagnosticLogs based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {
namespace EmberApplicationConfig {

using FeatureBitmapType = NoFeatureFlagsDefined;

inline constexpr std::array<EmberApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kEmberClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
    },
} };

} // namespace EmberApplicationConfig
} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace namespace chip

