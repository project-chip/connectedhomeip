// DO NOT EDIT - Generated file
//
// Application configuration for LaundryWasherControls based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace LaundryWasherControls {
namespace StaticApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kSpin, // feature bit 0x1
            FeatureBitmapType::kRinse// feature bit 0x2
        },
    },
} };

} // namespace StaticApplicationConfig
} // namespace LaundryWasherControls
} // namespace Clusters
} // namespace app
} // namespace namespace chip

