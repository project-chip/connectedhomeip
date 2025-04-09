// DO NOT EDIT - Generated file
//
// Application configuration for BooleanStateConfiguration based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace BooleanStateConfiguration {
namespace StaticApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<ClusterConfiguration<FeatureBitmapType>, 1> kClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kVisual, // feature bit 0x1
            FeatureBitmapType::kAudible, // feature bit 0x2
            FeatureBitmapType::kAlarmSuppress, // feature bit 0x4
            FeatureBitmapType::kSensitivityLevel// feature bit 0x8
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace BooleanStateConfiguration
} // namespace Clusters
} // namespace app
} // namespace namespace chip

