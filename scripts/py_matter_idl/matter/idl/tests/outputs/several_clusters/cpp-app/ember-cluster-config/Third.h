// DO NOT EDIT - Generated file
//
// Application configuration for Third based on EMBER configuration
// from inputs/several_clusters.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Third {
namespace EmberApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<EmberApplicationConfig::ClusteConfiguration<FeatureBitmapType>, 1> kEmberClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 2,
    },
} };

} // namespace EmberApplicationConfig
} // namespace Third
} // namespace Clusters
} // namespace app
} // namespace namespace chip

