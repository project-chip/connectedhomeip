// DO NOT EDIT - Generated file
//
// Application configuration for LocalizationConfiguration based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace LocalizationConfiguration {
namespace EmberApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<EmberApplicationConfig::ClusteConfiguration<FeatureBitmapType>, 1> kEmberClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 1,
    },
} };

} // namespace EmberApplicationConfig
} // namespace LocalizationConfiguration
} // namespace Clusters
} // namespace app
} // namespace namespace chip

