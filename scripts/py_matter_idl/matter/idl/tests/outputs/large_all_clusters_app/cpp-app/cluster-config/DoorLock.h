// DO NOT EDIT - Generated file
//
// Application configuration for DoorLock
#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/cluster-config.h>
#include <lib/support/BitFlags.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {
namespace ApplicationConfig {

using FeatureBitmapType = BitFlags<Feature>;

inline constexpr std::array<ClusterEndpointConfiguration<FeatureBitmapType>, 1> kFixedEndpoints = { {
    {
        .endpointNumber = 1,
        .featureMap = FeatureBitmapType {
            Feature::kPINCredentials, // feature bit 0x1
            Feature::kRFIDCredentials, // feature bit 0x2
            Feature::kWeekDaySchedules, // feature bit 0x10
            Feature::kUsersManagement, // feature bit 0x100
            Feature::kYearDaySchedules, // feature bit 0x400
            Feature::kHolidaySchedules // feature bit 0x800
        },
        .clusterRevision = 6,
    },
} };

} // namespace ApplicationConfig
} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace namespace chip

