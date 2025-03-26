// DO NOT EDIT - Generated file
//
// Application configuration for PumpConfigurationAndControl
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace PumpConfigurationAndControl {
namespace ApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<FeatureBitmapType, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 3,
    },
} };

} // namespace ApplicationConfig
} // namespace PumpConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace namespace chip

