// DO NOT EDIT - Generated file
//
// Application configuration for PumpConfigurationAndControl based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace PumpConfigurationAndControl {
namespace StaticApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kConstantPressure, // feature bit 0x1
            FeatureBitmapType::kCompensatedPressure, // feature bit 0x2
            FeatureBitmapType::kConstantFlow, // feature bit 0x4
            FeatureBitmapType::kConstantSpeed, // feature bit 0x8
            FeatureBitmapType::kConstantTemperature// feature bit 0x10
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace PumpConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace namespace chip

