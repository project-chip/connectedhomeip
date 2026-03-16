/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/zone-management-server/TwoDCartesianZoneStorage.h>
#include <app/clusters/zone-management-server/ZoneInformationStorage.h>
#include <protocols/interaction_model/StatusCode.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

class ZoneMgmtServer;

/** @brief
 *  Defines interfaces for implementing application-specific logic for various aspects of the ZoneManagement Cluster.
 *  Specifically, it defines interfaces for the command handling and loading of the allocated streams.
 */
class Delegate
{
public:
    Delegate() = default;

    virtual ~Delegate() = default;

    /**
     *    @brief Command Delegate for creation of TwoDCartesianZone with the provided parameters.
     *
     *   @param[in]  zone       Structure with parameters for defining a TwoDCartesian zone.
     *
     *   @param[out] outZoneID  Indicates the ID of the created zone.
     *
     *   @return Success if the creation is successful and a zoneID was
     *   produced; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     */
    virtual Protocols::InteractionModel::Status CreateTwoDCartesianZone(const TwoDCartesianZoneStorage & zone,
                                                                        uint16_t & outZoneID) = 0;

    /**
     *    @brief Command Delegate for updating of a TwoDCartesianZone with the provided parameters.
     *
     *   @param[in] zoneID  Indicates the ID of the zone to update.
     *   @param[in]  zone    Structure with parameters for a TwoDCartesian zone.
     *
     *
     *   @return Success if the update is successful; otherwise, the command SHALL be
     *   rejected with an appropriate error.
     */
    virtual Protocols::InteractionModel::Status UpdateTwoDCartesianZone(uint16_t zoneID, const TwoDCartesianZoneStorage & zone) = 0;

    /**
     *    @brief Command Delegate for the removal of a TwoDCartesianZone for a given zoneID.
     *
     *   @param[in] zoneID  Indicates the ID of the zone to remove.
     *
     *   @return Success if the removal is successful; otherwise, the command SHALL be
     *   rejected with an appropriate error.
     */
    virtual Protocols::InteractionModel::Status RemoveZone(uint16_t zoneID) = 0;

    /**
     *    @brief Command Delegate for creation of a ZoneTrigger.
     *
     *   @param[in]  zoneTrigger    Structure with parameters for defining a ZoneTriggerControl.
     *
     *   @return Success if the creation is successful; otherwise, the command SHALL be
     *   rejected with an appropriate error.
     */
    virtual Protocols::InteractionModel::Status CreateTrigger(const ZoneTriggerControlStruct & zoneTrigger) = 0;

    /**
     *    @brief Command Delegate for update of a ZoneTrigger.
     *
     *   @param[in]  zoneTrigger    Structure with parameters for defining a ZoneTriggerControl.
     *
     *   @return Success if the update is successful; otherwise, the command SHALL be
     *   rejected with an appropriate error.
     */
    virtual Protocols::InteractionModel::Status UpdateTrigger(const ZoneTriggerControlStruct & zoneTrigger) = 0;

    /**
     *    @brief Command Delegate for the removal of a ZoneTrigger for a given zoneID.
     *
     *   @param[in] zoneID  Indicates the ID of the zone to remove the ZoneTrigger for.
     *
     *   @return Success if the removal is successful; otherwise, the command SHALL be
     *   rejected with an appropriate error.
     */
    virtual Protocols::InteractionModel::Status RemoveTrigger(uint16_t zoneID) = 0;

    /**
     *   @brief Delegate callback for notifying change in an attribute.
     *
     */
    virtual void OnAttributeChanged(AttributeId attributeId) = 0;

    /**
     *  @brief Callback into the delegate once persistent attributes managed by
     *  the Cluster have been loaded from Storage.
     */
    virtual CHIP_ERROR PersistentAttributesLoadedCallback() = 0;

    /**
     * Delegate function to load the created zones and triggers.
     * The application is responsible for persisting them. The Load APIs
     * would be used to load the persisted zones and triggers into the cluster
     * server list members at initialization.
     * Once loaded, the cluster server can serve Reads on these
     * attributes.
     */
    virtual CHIP_ERROR LoadZones(std::vector<ZoneInformationStorage> & aZones) = 0;

    virtual CHIP_ERROR LoadTriggers(std::vector<ZoneTriggerControlStruct> & aTriggers) = 0;

    ZoneMgmtServer * GetZoneMgmtServer() const { return mZoneMgmtServer; }

private:
    friend class ZoneMgmtServer;

    ZoneMgmtServer * mZoneMgmtServer = nullptr;

    /**
     * This method is used by the SDK to ensure the delegate points to the server instance it's associated with.
     * When a server instance is created or destroyed, this method will be called to set and clear, respectively,
     * the pointer to the server instance.
     *
     * @param aZoneMgmtServer  A pointer to the ZoneMgmtServer object related to this delegate object.
     */
    void SetZoneMgmtServer(ZoneMgmtServer * aZoneMgmtServer) { mZoneMgmtServer = aZoneMgmtServer; }
};

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
