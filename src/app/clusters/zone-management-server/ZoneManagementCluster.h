/*
 *
 *    Copyright (c) 2025-2026 Project CHIP Authors
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
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

#include <optional>
#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

using TwoDCartesianZoneDecodableStruct = Structs::TwoDCartesianZoneStruct::DecodableType;
using TwoDCartesianZoneStruct          = Structs::TwoDCartesianZoneStruct::Type;
using TwoDCartesianVertexStruct        = Structs::TwoDCartesianVertexStruct::Type;
using ZoneInformationStruct            = Structs::ZoneInformationStruct::Type;
using ZoneTriggerControlStruct         = Structs::ZoneTriggerControlStruct::Type;

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

class ZoneManagementCluster;

class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    virtual Protocols::InteractionModel::Status CreateTwoDCartesianZone(const TwoDCartesianZoneStorage & zone,
                                                                        uint16_t & outZoneID)                                   = 0;
    virtual Protocols::InteractionModel::Status UpdateTwoDCartesianZone(uint16_t zoneID, const TwoDCartesianZoneStorage & zone) = 0;
    virtual Protocols::InteractionModel::Status RemoveZone(uint16_t zoneID)                                                     = 0;
    virtual Protocols::InteractionModel::Status CreateTrigger(const ZoneTriggerControlStruct & zoneTrigger)                     = 0;
    virtual Protocols::InteractionModel::Status UpdateTrigger(const ZoneTriggerControlStruct & zoneTrigger)                     = 0;
    virtual Protocols::InteractionModel::Status RemoveTrigger(uint16_t zoneID)                                                  = 0;
    virtual void OnAttributeChanged(AttributeId attributeId)                                                                    = 0;
    virtual CHIP_ERROR PersistentAttributesLoadedCallback()                                                                     = 0;
    virtual CHIP_ERROR LoadZones(std::vector<ZoneInformationStorage> & aZones)                                                  = 0;
    virtual CHIP_ERROR LoadTriggers(std::vector<ZoneTriggerControlStruct> & aTriggers)                                          = 0;

protected:
    friend class ZoneManagementCluster;
    // This is named mZoneManagementServer instead of mZoneManagementCluster to preserve backwards compatibility with legacy usage.
    ZoneManagementCluster * mZoneManagementServer = nullptr;

    void SetZoneManagementCluster(ZoneManagementCluster * zoneManagementCluster) { mZoneManagementServer = zoneManagementCluster; }
    ZoneManagementCluster * GetZoneManagementCluster() const { return mZoneManagementServer; }
};

class ZoneManagementCluster : public DefaultServerCluster
{
public:
    struct Context
    {
        Delegate & delegate;
        EndpointId endpointId;
        BitFlags<Feature> features;
        std::optional<uint8_t> initialSensitivity;
        struct Config
        {
            uint8_t maxUserDefinedZones;
            uint8_t maxZones;
            uint8_t sensitivityMax;
            TwoDCartesianVertexStruct twoDCartesianMax;
        } config;
    };

    /**
     * @brief Creates a Zone Management cluster instance from the supplied context.
     *
     * The caller must ensure that the referenced delegate remains valid for the lifetime of the cluster.
     * The provided endpoint, feature set, and configuration must match the device's supported Zone Management
     * cluster configuration.
     */
    ZoneManagementCluster(const Context & context);

    ~ZoneManagementCluster() override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType shutdownType) override;

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

    CHIP_ERROR SetSensitivity(uint8_t aSensitivity);

    const std::vector<ZoneInformationStorage> & GetZones() const { return mZones; }
    const std::vector<ZoneTriggerControlStruct> & GetTriggers() const { return mTriggers; }
    Optional<ZoneTriggerControlStruct> GetTriggerForZone(uint16_t zoneID) const;

    uint8_t GetMaxUserDefinedZones() const { return mClusterContext.config.maxUserDefinedZones; }
    uint8_t GetMaxZones() const { return mClusterContext.config.maxZones; }
    uint8_t GetSensitivityMax() const { return mClusterContext.config.sensitivityMax; }
    uint8_t GetSensitivity() const { return mSensitivity; }
    const TwoDCartesianVertexStruct & GetTwoDCartesianMax() const { return mClusterContext.config.twoDCartesianMax; }

    CHIP_ERROR AddZone(const ZoneInformationStorage & zone);
    CHIP_ERROR UpdateZone(uint16_t zoneId, const ZoneInformationStorage & zone);
    CHIP_ERROR RemoveZone(uint16_t zoneId);

    Protocols::InteractionModel::Status AddOrUpdateTrigger(const ZoneTriggerControlStruct & trigger);
    Protocols::InteractionModel::Status RemoveTrigger(uint16_t zoneId);

    Protocols::InteractionModel::Status GenerateZoneTriggeredEvent(uint16_t zoneID, ZoneEventTriggeredReasonEnum triggerReason);
    Protocols::InteractionModel::Status GenerateZoneStoppedEvent(uint16_t zoneID, ZoneEventStoppedReasonEnum stopReason);

private:
    const Context mClusterContext;
    Delegate & mDelegate;

    uint8_t mUserDefinedZonesCount = 0;

    std::vector<ZoneInformationStorage> mZones;
    std::vector<ZoneTriggerControlStruct> mTriggers;
    uint8_t mSensitivity = 1;

    CHIP_ERROR ValidateConfiguration() const;
    void LoadPersistentAttributes();

    CHIP_ERROR ReadAndEncodeZones(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR ReadAndEncodeTriggers(const AttributeValueEncoder::ListEncodeHelper & encoder);

    Protocols::InteractionModel::Status ValidateTwoDCartesianZone(const TwoDCartesianZoneDecodableStruct & zone);
    Protocols::InteractionModel::Status ValidateTrigger(const ZoneTriggerControlStruct & trigger);

    bool DoZoneUseAndVerticesMatch(ZoneUseEnum use, const std::vector<TwoDCartesianVertexStruct> & vertices,
                                   const TwoDCartesianZoneStorage & zone);
    bool ZoneAlreadyExists(ZoneUseEnum zoneUse, const std::vector<TwoDCartesianVertexStruct> & vertices,
                           const DataModel::Nullable<uint16_t> & excludeZoneId);

    std::optional<DataModel::ActionReturnStatus>
    HandleCreateTwoDCartesianZone(const ConcreteCommandPath & requestPath, CommandHandler * handler,
                                  const Commands::CreateTwoDCartesianZone::DecodableType & req);

    DataModel::ActionReturnStatus HandleUpdateTwoDCartesianZone(const Commands::UpdateTwoDCartesianZone::DecodableType & req);
    DataModel::ActionReturnStatus HandleRemoveZone(const Commands::RemoveZone::DecodableType & req);
    DataModel::ActionReturnStatus HandleCreateOrUpdateTrigger(const Commands::CreateOrUpdateTrigger::DecodableType & req);
    DataModel::ActionReturnStatus HandleRemoveTrigger(const Commands::RemoveTrigger::DecodableType & req);
};

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
