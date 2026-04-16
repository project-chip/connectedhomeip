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

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/StatusResponse.h>
#include <app/clusters/zone-management-server/Delegate.h>
#include <app/clusters/zone-management-server/TwoDCartesianZoneStorage.h>
#include <app/clusters/zone-management-server/ZoneInformationStorage.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <optional>
#include <protocols/interaction_model/StatusCode.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

class ZoneMgmtServer : public CommandHandlerInterface, public AttributeAccessInterface
{
public:
    /**
     * @brief Creates a Zone Management cluster instance. The Init() function needs to be called for this instance
     * to be registered and called by the interaction model at the appropriate times.
     *
     * @param aDelegate                         A reference to the delegate to be used by this server.
     *                                          Note: the caller must ensure that the delegate lives throughout the instance's
     *                                          lifetime.
     *
     * @param aEndpointId                       The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aFeatures                         The bitflags value that identifies which features are supported by this instance.
     * @param aMaxUserDefinedZones              The maximum number of user-defined zones supported by the device.
     *                                          This value is specified by the device manufacturer.
     * @param aMaxZones                         The maximum number of zones that are allowed to exist on the device. This is the
     *                                          sum of the predefined built-in zones and the user-defined zones.
     * @param aSensitivityMax                   The hardware-specific value for the number of supported sensitivity levels.
     *                                          This value is specified by the device manufacturer.
     * @param aTwoDCartesianMax                 The maximum X and Y points that are allowed for TwoD Cartesian Zones.
     *
     */
    ZoneMgmtServer(Delegate & aDelegate, EndpointId aEndpointId, const BitFlags<Feature> aFeatures, uint8_t aMaxUserDefinedZones,
                   uint8_t aMaxZones, uint8_t aSensitivityMax, const TwoDCartesianVertexStruct & aTwoDCartesianMax);

    ~ZoneMgmtServer() override;

    /**
     * @brief Initialise the Zone Management server instance.
     * This function must be called after defining a ZoneMgmtServer class object.
     * @return Returns an error if some of the constraint/feature validation checks fail or
     * the CommandHandler or AttributeHandler registration fails, else returns CHIP_NO_ERROR.
     */
    CHIP_ERROR Init();

    bool HasFeature(Feature feature) const;

    // Attribute Setters
    CHIP_ERROR SetSensitivity(uint8_t aSensitivity);

    // Attribute Getters
    const std::vector<ZoneInformationStorage> & GetZones() const { return mZones; }

    const std::vector<ZoneTriggerControlStruct> & GetTriggers() const { return mTriggers; }

    const Optional<ZoneTriggerControlStruct> GetTriggerForZone(uint16_t zoneID);

    uint8_t GetMaxUserDefinedZones() const { return mMaxUserDefinedZones; }
    uint8_t GetMaxZones() const { return mMaxZones; }
    uint8_t GetSensitivityMax() const { return mSensitivityMax; }
    uint8_t GetSensitivity() const { return mSensitivity; }
    const TwoDCartesianVertexStruct & GetTwoDCartesianMax() const { return mTwoDCartesianMax; }

    CHIP_ERROR AddZone(const ZoneInformationStorage & zone);
    CHIP_ERROR UpdateZone(uint16_t zoneId, const ZoneInformationStorage & zone);
    CHIP_ERROR RemoveZone(uint16_t zoneId);

    Protocols::InteractionModel::Status AddOrUpdateTrigger(const ZoneTriggerControlStruct & trigger);
    Protocols::InteractionModel::Status RemoveTrigger(uint16_t zoneId);

    // Generate Zone events
    Protocols::InteractionModel::Status GenerateZoneTriggeredEvent(uint16_t zoneID, ZoneEventTriggeredReasonEnum triggerReason);
    Protocols::InteractionModel::Status GenerateZoneStoppedEvent(uint16_t zoneID, ZoneEventStoppedReasonEnum stopReason);

private:
    Delegate & mDelegate;
    EndpointId mEndpointId;
    const BitFlags<Feature> mFeatures;

    // Attributes
    const uint8_t mMaxUserDefinedZones;
    const uint8_t mMaxZones;
    const uint8_t mSensitivityMax;
    const TwoDCartesianVertexStruct mTwoDCartesianMax;
    uint8_t mUserDefinedZonesCount = 0;

    std::vector<ZoneInformationStorage> mZones;
    std::vector<ZoneTriggerControlStruct> mTriggers;
    uint8_t mSensitivity = 0;

    /**
     * IM-level implementation of read
     * @return appropriately mapped CHIP_ERROR if applicable
     */
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    /**
     * IM-level implementation of write
     * @return appropriately mapped CHIP_ERROR if applicable
     */
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    /**
     * Helper function that loads all the persistent attributes from the KVS.
     */
    void LoadPersistentAttributes();

    CHIP_ERROR ReadAndEncodeZones(const AttributeValueEncoder::ListEncodeHelper & encoder);

    CHIP_ERROR ReadAndEncodeTriggers(const AttributeValueEncoder::ListEncodeHelper & encoder);

    Protocols::InteractionModel::Status ValidateTwoDCartesianZone(const TwoDCartesianZoneDecodableStruct & zone);

    Protocols::InteractionModel::Status ValidateTrigger(const ZoneTriggerControlStruct & trigger);

    // Utility that matches a given zone's ZoneUse and verices with the given
    // parameters to check if they match. Used by ZoneAlreadyExists().
    bool DoZoneUseAndVerticesMatch(ZoneUseEnum use, const std::vector<TwoDCartesianVertexStruct> & vertices,
                                   const TwoDCartesianZoneStorage & zone);

    // Utility function to check if a given ZoneUse and a TwoDVertex already
    // exists in mZones.
    bool ZoneAlreadyExists(ZoneUseEnum zoneUse, const std::vector<TwoDCartesianVertexStruct> & vertices,
                           const DataModel::Nullable<uint16_t> & excludeZoneId);

    /**
     * @brief Inherited from CommandHandlerInterface
     */
    void InvokeCommand(HandlerContext & ctx) override;

    void HandleCreateTwoDCartesianZone(HandlerContext & ctx, const Commands::CreateTwoDCartesianZone::DecodableType & req);

    void HandleUpdateTwoDCartesianZone(HandlerContext & ctx, const Commands::UpdateTwoDCartesianZone::DecodableType & req);

    void HandleRemoveZone(HandlerContext & ctx, const Commands::RemoveZone::DecodableType & req);

    void HandleCreateOrUpdateTrigger(HandlerContext & ctx, const Commands::CreateOrUpdateTrigger::DecodableType & req);

    void HandleRemoveTrigger(HandlerContext & ctx, const Commands::RemoveTrigger::DecodableType & req);
};

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
