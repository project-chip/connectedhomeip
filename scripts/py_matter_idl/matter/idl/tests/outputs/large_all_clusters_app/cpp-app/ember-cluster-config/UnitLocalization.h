// DO NOT EDIT - Generated file
//
// Application configuration for UnitLocalization based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace UnitLocalization {
namespace EmberApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<ClusteConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kTemperatureUnit// feature bit 0x1
        },
        .clusterRevision = 1,
    },
} };

} // namespace EmberApplicationConfig
} // namespace UnitLocalization
} // namespace Clusters
} // namespace app
} // namespace namespace chip

