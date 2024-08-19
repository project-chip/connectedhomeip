/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <app/CommandHandler.h>
#include <app/clusters/occupancy-sensor-server/occupancy-hal.h>
#include <app/clusters/occupancy-sensor-server/occupancy-sensor-server.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OccupancySensing;
using namespace chip::app::Clusters::OccupancySensing::Structs;
using namespace chip::DeviceLayer;

using chip::Protocols::InteractionModel::Status;

namespace {

static constexpr size_t kOccupancySensingClusterTableSize =
    MATTER_DM_OCCUPANCY_SENSING_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kOccupancySensingClusterTableSize <= kEmberInvalidEndpointIndex, "Occupancy Sensing Cluster table size error");

static std::unique_ptr<Instance> gOccupancySensingClusterInstances[kOccupancySensingClusterTableSize];

} // namespace

void emberAfOccupancySensingClusterInitCallback(EndpointId endpointId)
{
    uint16_t epIndex = emberAfGetClusterServerEndpointIndex(endpointId, chip::app::Clusters::OccupancySensing::Id,
                                                            MATTER_DM_OCCUPANCY_SENSING_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (epIndex < kOccupancySensingClusterTableSize)
    {
        VerifyOrDie(!gOccupancySensingClusterInstances[epIndex]);

        gOccupancySensingClusterInstances[epIndex] =
            std::make_unique<Instance>(BitMask<OccupancySensing::Feature, uint32_t>(OccupancySensing::Feature::kPassiveInfrared));

        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimits = {
            .holdTimeMin     = 1,
            .holdTimeMax     = 300,
            .holdTimeDefault = 10,
        };

        uint16_t holdTime = 10;

        if (gOccupancySensingClusterInstances[epIndex])
        {
            gOccupancySensingClusterInstances[epIndex]->Init();

            SetHoldTimeLimits(endpointId, holdTimeLimits);

            SetHoldTime(endpointId, holdTime);
        }
    }
    else
    {
        ChipLogError(AppServer, "Error: invalid/unexpected OccupancySensing Cluster endpoint index.");
    }
}
