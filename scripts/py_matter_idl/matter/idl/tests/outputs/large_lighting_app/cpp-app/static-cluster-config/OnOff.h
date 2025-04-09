// DO NOT EDIT - Generated file
//
// Application configuration for OnOff based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace OnOff {
namespace EmberApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<EmberApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kEmberClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kLighting// feature bit 0x1
        },
    },
} };

} // namespace EmberApplicationConfig
} // namespace OnOff
} // namespace Clusters
} // namespace app
} // namespace namespace chip

