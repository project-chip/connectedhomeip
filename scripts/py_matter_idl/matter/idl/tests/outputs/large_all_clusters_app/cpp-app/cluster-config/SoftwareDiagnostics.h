// DO NOT EDIT - Generated file
//
// Application configuration for SoftwareDiagnostics
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace SoftwareDiagnostics {
namespace ApplicationConfig {

using FeatureBitmapType = BitFlags<Feature>;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 0,
        .featureMap = FeatureBitmapType {
            Feature::kWaterMarks // feature bit 0x1
        },
        .clusterRevision = 1,
    },
} };

} // namespace ApplicationConfig
} // namespace SoftwareDiagnostics
} // namespace Clusters
} // namespace app
} // namespace namespace chip

