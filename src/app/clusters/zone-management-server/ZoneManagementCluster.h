/*
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

#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/ZoneManagement/Attributes.h>
#include <clusters/ZoneManagement/Commands.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

using TwoDCartesianZoneDecodableStruct = Structs::TwoDCartesianZoneStruct::DecodableType;
using TwoDCartesianZoneStruct          = Structs::TwoDCartesianZoneStruct::Type;
using TwoDCartesianVertexStruct        = Structs::TwoDCartesianVertexStruct::Type;
using ZoneInformationStruct            = Structs::ZoneInformationStruct::Type;
using ZoneTriggerControlStruct         = Structs::ZoneTriggerControlStruct::Type;

class ZoneManagementCluster;

struct TwoDCartesianZoneStorage : TwoDCartesianZoneStruct
{
    TwoDCartesianZoneStorage(){};

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
    ZoneInformationStorage(){};

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
     *   @param[in] zoneID  Indicates the ID of the zone to create.
     *   @param[in]  zone       Structure with parameters for defining a TwoDCartesian zone.
     *
     *   @return Success if the creation is successful; otherwise, the command
     *   SHALL be rejected with an appropriate error.
     */
    virtual Protocols::InteractionModel::Status CreateTwoDCartesianZone(uint16_t zoneID, const TwoDCartesianZoneStorage & zone) = 0;

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

    ZoneManagementCluster * GetZoneManagementCluster() const { return mZoneMgmtServer; }

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
    ZoneManagementCluster(EndpointId endpointId, Delegate & delegate, const BitFlags<Feature> features, uint8_t maxUserDefinedZones,
                          uint8_t maxZones, uint8_t sensitivityMax, uint8_t sensitivity,
                          const TwoDCartesianVertexStruct & twoDCartesianMax);
    ~ZoneManagementCluster() override;

    CHIP_ERROR Init();

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    bool HasFeature(Feature feature) const;

    // Attribute Getters
    const std::vector<ZoneInformationStorage> & GetZones() const { return mZones; }

    const std::vector<ZoneTriggerControlStruct> & GetTriggers() const { return mTriggers; }

    const Optional<ZoneTriggerControlStruct> GetTriggerForZone(uint16_t zoneID);

    uint16_t GetNewZoneId();

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
    const BitFlags<Feature> mFeatures;

    // Attributes
    const uint8_t mMaxUserDefinedZones;
    const uint8_t mMaxZones;
    const uint8_t mSensitivityMax;
    uint8_t mSensitivity;
    const TwoDCartesianVertexStruct mTwoDCartesianMax;
    uint8_t mUserDefinedZonesCount = 0;

    std::vector<ZoneInformationStorage> mZones;
    std::vector<ZoneTriggerControlStruct> mTriggers;

    /**
     * Helper function that loads all the persistent attributes from the KVS.
     */
    CHIP_ERROR LoadZones();
    CHIP_ERROR LoadTriggers();
    CHIP_ERROR LoadSensitivity();

    CHIP_ERROR PersistZones();
    CHIP_ERROR PersistTriggers();

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

    void HandleCreateTwoDCartesianZone(CommandHandler * handler, const ConcreteCommandPath & commandPath,
                                       const Commands::CreateTwoDCartesianZone::DecodableType & req);

    void HandleUpdateTwoDCartesianZone(CommandHandler * handler, const ConcreteCommandPath & commandPath,
                                       const Commands::UpdateTwoDCartesianZone::DecodableType & req);

    void HandleRemoveZone(CommandHandler * handler, const ConcreteCommandPath & commandPath,
                          const Commands::RemoveZone::DecodableType & req);

    void HandleCreateOrUpdateTrigger(CommandHandler * handler, const ConcreteCommandPath & commandPath,
                                     const Commands::CreateOrUpdateTrigger::DecodableType & req);

    void HandleRemoveTrigger(CommandHandler * handler, const ConcreteCommandPath & commandPath,
                             const Commands::RemoveTrigger::DecodableType & req);
};

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
