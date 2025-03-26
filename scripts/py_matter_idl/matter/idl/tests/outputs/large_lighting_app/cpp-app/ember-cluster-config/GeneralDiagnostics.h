// DO NOT EDIT - Generated file
//
// Application configuration for GeneralDiagnostics based on EMBER configuration
// from inputs/large_lighting_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralDiagnostics {
namespace EmberApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<ClusteConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 1,
    },
} };

} // namespace EmberApplicationConfig
} // namespace GeneralDiagnostics
} // namespace Clusters
} // namespace app
} // namespace namespace chip

