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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

using TwoDCartesianZoneDecodableStruct = Structs::TwoDCartesianZoneStruct::DecodableType;
using TwoDCartesianZoneStruct          = Structs::TwoDCartesianZoneStruct::Type;
using TwoDCartesianVertexStruct        = Structs::TwoDCartesianVertexStruct::Type;
using ZoneInformationStruct            = Structs::ZoneInformationStruct::Type;
using ZoneTriggerControlStruct         = Structs::ZoneTriggerControlStruct::Type;
using ActionReturnStatus               = DataModel::ActionReturnStatus;
using AttributeEntry                   = DataModel::AttributeEntry;
using AcceptedCommandEntry             = DataModel::AcceptedCommandEntry;

class ZoneManagementCluster;

struct TwoDCartesianZoneStorage : TwoDCartesianZoneStruct
{
    TwoDCartesianZoneStorage() = default;

    TwoDCartesianZoneStorage(const CharSpan & aName, ZoneUseEnum aUse, const std::vector<TwoDCartesianVertexStruct> & aVertices,
                             Optional<CharSpan> aColor)
    {
        Set(aName, aUse, aVertices, aColor);
    }

    TwoDCartesianZoneStorage(const TwoDCartesianZoneStorage & aTwoDCartZone) { *this = aTwoDCartZone; }

    TwoDCartesianZoneStorage & operator=(const TwoDCartesianZoneStorage & aTwoDCartZone)
    {
        Set(aTwoDCartZone.name, aTwoDCartZone.use, aTwoDCartZone.verticesVector, aTwoDCartZone.color);
        return *this;
    }

    void Set(const CharSpan & aName, ZoneUseEnum aUse, const std::vector<TwoDCartesianVertexStruct> & aVertices,
             Optional<CharSpan> aColor)
    {
        nameString     = std::string(aName.begin(), aName.end());
        name           = CharSpan(nameString.c_str(), nameString.size());
        use            = aUse;
        verticesVector = aVertices;
        vertices       = DataModel::List<TwoDCartesianVertexStruct>(verticesVector.data(), verticesVector.size());
        if (aColor.HasValue())
        {
            colorString = std::string(aColor.Value().begin(), aColor.Value().end());
            color       = MakeOptional(CharSpan(colorString.c_str(), colorString.size()));
        }
        else
        {
            colorString.clear();
            color = NullOptional;
        }
    }

    std::string nameString;
    std::string colorString;
    std::vector<TwoDCartesianVertexStruct> verticesVector;
};

struct ZoneInformationStorage : ZoneInformationStruct
{
    ZoneInformationStorage() = default;

    ZoneInformationStorage(const uint16_t & aZoneID, ZoneTypeEnum aZoneType, ZoneSourceEnum aZoneSource,
                           const Optional<TwoDCartesianZoneStorage> & aTwoDCartZoneStorage)
    {
        Set(aZoneID, aZoneType, aZoneSource, aTwoDCartZoneStorage);
    }

    ZoneInformationStorage(const ZoneInformationStorage & aZoneInfoStorage) { *this = aZoneInfoStorage; }

    ZoneInformationStorage & operator=(const ZoneInformationStorage & aZoneInfoStorage)
    {
        Set(aZoneInfoStorage.zoneID, aZoneInfoStorage.zoneType, aZoneInfoStorage.zoneSource, aZoneInfoStorage.twoDCartZoneStorage);
        return *this;
    }

    void Set(const uint16_t & aZoneID, ZoneTypeEnum aZoneType, ZoneSourceEnum aZoneSource,
             const Optional<TwoDCartesianZoneStorage> & aTwoDCartZoneStorage)
    {
        zoneID              = aZoneID;
        zoneType            = aZoneType;
        zoneSource          = aZoneSource;
        twoDCartZoneStorage = aTwoDCartZoneStorage;
        twoDCartesianZone   = twoDCartZoneStorage;
    }

    Optional<TwoDCartesianZoneStorage> twoDCartZoneStorage;
};

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

    ZoneManagementCluster * GetZoneMgmtServer() const { return mZoneMgmtServer; }

private:
    friend class ZoneManagementCluster;

    ZoneManagementCluster * mZoneMgmtServer = nullptr;

    /**
     * This method is used by the SDK to ensure the delegate points to the server instance it's associated with.
     * When a server instance is created or destroyed, this method will be called to set and clear, respectively,
     * the pointer to the server instance.
     *
     * @param aZoneMgmtServer  A pointer to the ZoneMgmtServer object related to this delegate object.
     */
    void SetZoneMgmtServer(ZoneManagementCluster * aZoneMgmtServer) { mZoneMgmtServer = aZoneMgmtServer; }
};

class ZoneManagementCluster : public DefaultServerCluster
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
    ZoneManagementCluster(Delegate & aDelegate, EndpointId aEndpointId, const BitFlags<Feature> aFeatures,
                          uint8_t aMaxUserDefinedZones, uint8_t aMaxZones, uint8_t aSensitivityMax,
                          const TwoDCartesianVertexStruct & aTwoDCartesianMax);

    ~ZoneManagementCluster() override;

    /**
     * @brief Initialise the Zone Management server instance.
     * This function must be called after defining a ZoneMgmtServer class object.
     * @return Returns an error if some of the constraint/feature validation checks fail or
     * the CommandHandler or AttributeHandler registration fails, else returns CHIP_NO_ERROR.
     */
    CHIP_ERROR Init();

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType shutdownType) override;

    ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request, AttributeValueEncoder & encoder) override;
    ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder) override;

    std::optional<ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                    CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

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
    ServerClusterRegistration mRegistration;

    const BitFlags<Feature> mFeatures;

    // Attributes
    const uint8_t mMaxUserDefinedZones;
    const uint8_t mMaxZones;
    const uint8_t mSensitivityMax;
    const TwoDCartesianVertexStruct mTwoDCartesianMax;
    uint8_t mUserDefinedZonesCount = 0;

    bool mSensitivityConfiguredByApp = false;
    bool mIsRegistered               = false;
    std::vector<ZoneInformationStorage> mZones;
    std::vector<ZoneTriggerControlStruct> mTriggers;
    uint8_t mSensitivity = 1;

    CHIP_ERROR ValidateConfiguration() const;
    /**
     * Helper function that loads all the persistent attributes from the KVS.
     */
    void LoadPersistentAttributes();
    void NotifyClusterAttributeChanged(AttributeId attributeId);

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

    std::optional<ActionReturnStatus> HandleCreateTwoDCartesianZone(const ConcreteCommandPath & requestPath,
                                                                    CommandHandler * handler,
                                                                    const Commands::CreateTwoDCartesianZone::DecodableType & req);

    ActionReturnStatus HandleUpdateTwoDCartesianZone(const Commands::UpdateTwoDCartesianZone::DecodableType & req);
    ActionReturnStatus HandleRemoveZone(const Commands::RemoveZone::DecodableType & req);
    ActionReturnStatus HandleCreateOrUpdateTrigger(const Commands::CreateOrUpdateTrigger::DecodableType & req);
    ActionReturnStatus HandleRemoveTrigger(const Commands::RemoveTrigger::DecodableType & req);
};

using ZoneMgmtServer = ZoneManagementCluster;

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
