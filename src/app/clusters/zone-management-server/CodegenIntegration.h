/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#include <app/clusters/zone-management-server/ZoneManagementCluster.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

class ZoneMgmtServer
{
public:
    ZoneMgmtServer(Delegate & delegate, EndpointId endpointId, BitFlags<Feature> features, uint8_t maxUserDefinedZones,
                   uint8_t maxZones, uint8_t sensitivityMax, const TwoDCartesianVertexStruct & twoDCartesianMax);
    ~ZoneMgmtServer();

    CHIP_ERROR Init();
    void Deinit();

    uint8_t GetSensitivity() const
    {
        if (mCluster.IsConstructed())
        {
            return mCluster.Cluster().GetSensitivity();
        }
        return mPendingAppSensitivity.value_or(1);
    }

    Optional<ZoneTriggerControlStruct> GetTriggerForZone(uint16_t zoneId) const;

    Protocols::InteractionModel::Status GenerateZoneTriggeredEvent(uint16_t zoneID, ZoneEventTriggeredReasonEnum triggerReason);
    Protocols::InteractionModel::Status GenerateZoneStoppedEvent(uint16_t zoneID, ZoneEventStoppedReasonEnum stopReason);

    CHIP_ERROR SetSensitivity(uint8_t sensitivity);

    chip::app::LazyRegisteredServerCluster<ZoneManagementCluster> mCluster;

private:
    const EndpointId mEndpointId;
    Delegate & mDelegate;
    const BitFlags<Feature> mFeatures;
    const ZoneManagementCluster::Context::Config mConfig;

    std::optional<uint8_t> mPendingAppSensitivity;
};

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
