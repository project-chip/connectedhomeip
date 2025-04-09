// DO NOT EDIT - Generated file
//
// Application configuration for Switch based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace Switch {
namespace StaticApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 3> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kLatchingSwitch// feature bit 0x1
        },
    },
    {
        .endpointNumber = 3,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kMomentarySwitch, // feature bit 0x2
            FeatureBitmapType::kMomentarySwitchLongPress, // feature bit 0x8
            FeatureBitmapType::kMomentarySwitchMultiPress, // feature bit 0x10
            FeatureBitmapType::kActionSwitch// feature bit 0x20
        },
    },
    {
        .endpointNumber = 4,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kMomentarySwitch, // feature bit 0x2
            FeatureBitmapType::kMomentarySwitchRelease, // feature bit 0x4
            FeatureBitmapType::kMomentarySwitchLongPress, // feature bit 0x8
            FeatureBitmapType::kMomentarySwitchMultiPress// feature bit 0x10
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace Switch
} // namespace Clusters
} // namespace app
} // namespace namespace chip

