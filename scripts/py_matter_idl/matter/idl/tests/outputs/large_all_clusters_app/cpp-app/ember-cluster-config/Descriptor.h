// DO NOT EDIT - Generated file
//
// Application configuration for Descriptor based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Descriptor {
namespace EmberApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<EmberApplicationConfig::ClusteConfiguration<FeatureBitmapType>, 4> kEmberClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 1,
    },
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 1,
    },
    {
        .endpointNumber = 2,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 1,
    },
    {
        .endpointNumber = 65534,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 1,
    },
} };

} // namespace EmberApplicationConfig
} // namespace Descriptor
} // namespace Clusters
} // namespace app
} // namespace namespace chip

