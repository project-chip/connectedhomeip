// DO NOT EDIT - Generated file
//
// Application configuration for ThermostatUserInterfaceConfiguration
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace ThermostatUserInterfaceConfiguration {
namespace ApplicationConfig {

using FeatureBitmapType = uint32_t;

inline constexpr std::array<FeatureBitmapType, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .clusterRevision = 2,
    },
} };

} // namespace ApplicationConfig
} // namespace ThermostatUserInterfaceConfiguration
} // namespace Clusters
} // namespace app
} // namespace namespace chip

