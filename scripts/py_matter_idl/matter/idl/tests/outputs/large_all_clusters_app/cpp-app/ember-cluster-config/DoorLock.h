// DO NOT EDIT - Generated file
//
// Application configuration for DoorLock based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {
namespace EmberApplicationConfig {

using FeatureBitmapType = Feature;

inline constexpr std::array<EmberApplicationConfig::ClusteConfiguration<FeatureBitmapType>, 1> kEmberClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
            FeatureBitmapType::kPINCredentials, // feature bit 0x1
            FeatureBitmapType::kRFIDCredentials, // feature bit 0x2
            FeatureBitmapType::kWeekDaySchedules, // feature bit 0x10
            FeatureBitmapType::kUsersManagement, // feature bit 0x100
            FeatureBitmapType::kYearDaySchedules, // feature bit 0x400
            FeatureBitmapType::kHolidaySchedules// feature bit 0x800
        },
    },
} };

} // namespace EmberApplicationConfig
} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace namespace chip

