// DO NOT EDIT - Generated file
//
// Application configuration for ModeSelect
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {
namespace ApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<FeatureBitmapType, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kDeponoff// feature bit 0x1
        },
        .clusterRevision = 1,
    },
} };

} // namespace ApplicationConfig
} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace namespace chip

