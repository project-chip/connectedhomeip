// DO NOT EDIT - Generated file
//
// Application configuration for SoftwareDiagnostics based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace SoftwareDiagnostics {
namespace EmberApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<EmberApplicationConfig::ClusteConfiguration<FeatureBitmapType>, 1> kEmberClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kWaterMarks// feature bit 0x1
        },
        .clusterRevision = 1,
    },
} };

} // namespace EmberApplicationConfig
} // namespace SoftwareDiagnostics
} // namespace Clusters
} // namespace app
} // namespace namespace chip

