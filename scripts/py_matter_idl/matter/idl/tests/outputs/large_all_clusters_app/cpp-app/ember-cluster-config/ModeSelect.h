// DO NOT EDIT - Generated file
//
// Application configuration for ModeSelect based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {
namespace EmberApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<ClusteConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kDeponoff// feature bit 0x1
        },
        .clusterRevision = 1,
    },
} };

} // namespace EmberApplicationConfig
} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace namespace chip

