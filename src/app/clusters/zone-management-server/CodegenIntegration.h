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
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

class ZoneMgmtServer
{
public:
    /**
     * @brief Creates a ZoneManagementCluster instance. The Init() function needs to be called for this instance
     * to be registered and called by the interaction model at the appropriate times.
     *
     * @param aDelegate                         A reference to the delegate to be used by this cluster.
     *                                          Note: the caller must ensure that the delegate lives throughout the instance's
     *                                          lifetime.
     *
     * @param aEndpointId                       The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aFeatures                         The bitflags value that identifies which features are supported by this instance.
     * @param aMaxUserDefinedZones               The maximum number of user-defined zones supported by the device.
     *                                          This value is specified by the device manufacturer.
     * @param aMaxZones                         The maximum number of zones that are allowed to exist on the device. This is the
     *                                          sum of the predefined built-in zones and the user-defined zones.
     * @param aSensitivityMax                   The hardware-specific value for the number of supported sensitivity levels.
     *                                          This value is specified by the device manufacturer.
     * @param aTwoDCartesianMax                 The maximum X and Y points that are allowed for TwoD Cartesian Zones.
     *
     */
    ZoneMgmtServer(Delegate & delegate, EndpointId endpointId, BitFlags<Feature> features, uint8_t maxUserDefinedZones,
                   uint8_t maxZones, uint8_t sensitivityMax, const TwoDCartesianVertexStruct & twoDCartesianMax);
    ~ZoneMgmtServer();

    /**
     * @brief Initialise the Zone Management server instance.
     * This function must be called after defining a ZoneMgmtServer class object.
     * @return Returns an error if some of the constraint/feature validation checks fail or
     * the CommandHandler or AttributeHandler registration fails, else returns CHIP_NO_ERROR.
     */
    CHIP_ERROR Init();
    void Deinit();

    uint8_t GetSensitivity() const;

    const std::vector<ZoneInformationStorage> & GetZones() const;

    const std::vector<ZoneTriggerControlStruct> & GetTriggers() const;

    Optional<ZoneTriggerControlStruct> GetTriggerForZone(uint16_t zoneId) const;

    uint8_t GetMaxUserDefinedZones() const;
    uint8_t GetMaxZones() const;
    uint8_t GetSensitivityMax() const;
    const TwoDCartesianVertexStruct & GetTwoDCartesianMax() const;

    CHIP_ERROR AddZone(const ZoneInformationStorage & zone);
    CHIP_ERROR UpdateZone(uint16_t zoneId, const ZoneInformationStorage & zone);
    CHIP_ERROR RemoveZone(uint16_t zoneId);

    Protocols::InteractionModel::Status AddOrUpdateTrigger(const ZoneTriggerControlStruct & trigger);
    Protocols::InteractionModel::Status RemoveTrigger(uint16_t zoneId);

    Protocols::InteractionModel::Status GenerateZoneTriggeredEvent(uint16_t zoneID, ZoneEventTriggeredReasonEnum triggerReason);
    Protocols::InteractionModel::Status GenerateZoneStoppedEvent(uint16_t zoneID, ZoneEventStoppedReasonEnum stopReason);

    CHIP_ERROR SetSensitivity(uint8_t sensitivity);

    ZoneManagementCluster & Cluster() { return mCluster.Cluster(); }

private:
    const EndpointId mEndpointId;
    Delegate & mDelegate;
    const BitFlags<Feature> mFeatures;
    const ZoneManagementCluster::Context::Config mConfig;
    chip::app::LazyRegisteredServerCluster<ZoneManagementCluster> mCluster;
    std::optional<uint8_t> mPendingAppSensitivity;
};

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
