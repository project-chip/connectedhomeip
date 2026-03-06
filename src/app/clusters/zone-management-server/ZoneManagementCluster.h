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

#include <app/server-cluster/DefaultServerCluster.h>

#include <optional>

// Shared storage and delegate definitions are kept in ZoneManagementCommon.h so
// the code-driven cluster can use them without depending on the legacy
// ZoneMgmtServer compatibility wrapper.
#include "ZoneManagementCommon.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

class ZoneManagementCluster : public DefaultServerCluster
{
    /**
     * @brief Creates a Zone Management cluster instance from the supplied context.
     *
     * The caller must ensure that the referenced delegate remains valid for the lifetime of the cluster.
     * The provided endpoint, feature set, and configuration must match the device's supported Zone Management
     * cluster configuration.
     */
public:
    struct Context {
        Delegate & delegate;
        EndpointId endpointId;
        BitFlags<Feature> features;

        struct Config {
            uint8_t maxUserDefinedZones;
            uint8_t maxZones;
            uint8_t sensitivityMax;
            TwoDCartesianVertexStruct twoDCartesianMax;
        } config;
    };

    ZoneManagementCluster(const Context & context);

    ~ZoneManagementCluster() override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    CHIP_ERROR Startup(ServerClusterContext & context, std::optional<uint8_t> initialSensitivity);
    void Shutdown(ClusterShutdownType shutdownType) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                               AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                AttributeValueDecoder & decoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    bool HasFeature(Feature feature) const;

    CHIP_ERROR SetSensitivity(uint8_t aSensitivity);

    const std::vector<ZoneInformationStorage> & GetZones() const { return mZones; }
    const std::vector<ZoneTriggerControlStruct> & GetTriggers() const { return mTriggers; }
    Optional<ZoneTriggerControlStruct> GetTriggerForZone(uint16_t zoneID) const;

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

    Protocols::InteractionModel::Status GenerateZoneTriggeredEvent(uint16_t zoneID, ZoneEventTriggeredReasonEnum triggerReason);
    Protocols::InteractionModel::Status GenerateZoneStoppedEvent(uint16_t zoneID, ZoneEventStoppedReasonEnum stopReason);

private:
    Context mClusterContext;
    Delegate & mDelegate;

    const BitFlags<Feature> mFeatures;
    const uint8_t mMaxUserDefinedZones;
    const uint8_t mMaxZones;
    const uint8_t mSensitivityMax;
    const TwoDCartesianVertexStruct mTwoDCartesianMax;
    uint8_t mUserDefinedZonesCount = 0;

    std::vector<ZoneInformationStorage> mZones;
    std::vector<ZoneTriggerControlStruct> mTriggers;
    uint8_t mSensitivity = 1;

    CHIP_ERROR ValidateConfiguration() const;
    void LoadPersistentAttributes(std::optional<uint8_t> initialSensitivity);

    CHIP_ERROR ReadAndEncodeZones(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR ReadAndEncodeTriggers(const AttributeValueEncoder::ListEncodeHelper & encoder);

    Protocols::InteractionModel::Status ValidateTwoDCartesianZone(const TwoDCartesianZoneDecodableStruct & zone);
    Protocols::InteractionModel::Status ValidateTrigger(const ZoneTriggerControlStruct & trigger);

    bool DoZoneUseAndVerticesMatch(ZoneUseEnum use, const std::vector<TwoDCartesianVertexStruct> & vertices,
                                   const TwoDCartesianZoneStorage & zone);
    bool ZoneAlreadyExists(ZoneUseEnum zoneUse, const std::vector<TwoDCartesianVertexStruct> & vertices,
                           const DataModel::Nullable<uint16_t> & excludeZoneId);

    std::optional<DataModel::ActionReturnStatus> HandleCreateTwoDCartesianZone(
        const ConcreteCommandPath & requestPath, CommandHandler * handler,
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
