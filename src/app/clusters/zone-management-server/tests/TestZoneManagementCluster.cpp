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

#include <app/clusters/zone-management-server/ZoneManagementCluster.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/ZoneManagement/Events.h>
#include <clusters/ZoneManagement/Metadata.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ZoneManagement;
using chip::Testing::ClusterTester;
using chip::Testing::EqualAcceptedCommandSets;
using chip::Testing::EqualGeneratedCommandSets;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;
using chip::Testing::IsGeneratedCommandsListEqualTo;

namespace {

constexpr EndpointId kTestEndpointId   = 1;
constexpr uint8_t kMaxUserDefinedZones = 5;
constexpr uint8_t kMaxZones            = 8;
constexpr uint8_t kSensitivityMax      = 4;
const TwoDCartesianVertexStruct kTwoDMaxPoint{ 640, 480 };

class MockDelegate : public Delegate
{
public:
    Protocols::InteractionModel::Status CreateTwoDCartesianZone(const TwoDCartesianZoneStorage & zone,
                                                                uint16_t & outZoneID) override
    {
        return CreateTwoDCartesianZone(zone, NullOptional, NullOptional, outZoneID);
    }

    Protocols::InteractionModel::Status CreateTwoDCartesianZone(const TwoDCartesianZoneStorage & zone,
                                                                const Optional<NodeId> & nodeId,
                                                                const Optional<EndpointId> & endpointId,
                                                                uint16_t & outZoneID) override
    {
        outZoneID = mNextZoneId++;

        ZoneInformationStorage zoneInfo;
        zoneInfo.Set(outZoneID, ZoneTypeEnum::kTwoDCARTZone, ZoneSourceEnum::kUser, MakeOptional(zone), nodeId, endpointId);
        mPersistedZones.push_back(zoneInfo);
        mCreateZoneCalls++;
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status UpdateTwoDCartesianZone(uint16_t zoneId, const TwoDCartesianZoneStorage & zone) override
    {
        return UpdateTwoDCartesianZone(zoneId, zone, NullOptional, NullOptional);
    }

    Protocols::InteractionModel::Status UpdateTwoDCartesianZone(uint16_t zoneId, const TwoDCartesianZoneStorage & zone,
                                                                const Optional<NodeId> & nodeId,
                                                                const Optional<EndpointId> & endpointId) override
    {
        for (auto & existing : mPersistedZones)
        {
            if (existing.zoneID == zoneId)
            {
                existing.Set(zoneId, ZoneTypeEnum::kTwoDCARTZone, ZoneSourceEnum::kUser, MakeOptional(zone), nodeId, endpointId);
                mUpdateZoneCalls++;
                return Protocols::InteractionModel::Status::Success;
            }
        }

        return Protocols::InteractionModel::Status::NotFound;
    }

    Protocols::InteractionModel::Status RemoveZone(uint16_t zoneId) override
    {
        const size_t originalSize = mPersistedZones.size();
        mPersistedZones.erase(std::remove_if(mPersistedZones.begin(), mPersistedZones.end(),
                                             [zoneId](const ZoneInformationStorage & zone) { return zone.zoneID == zoneId; }),
                              mPersistedZones.end());

        if (mPersistedZones.size() == originalSize)
        {
            return Protocols::InteractionModel::Status::NotFound;
        }

        mRemoveZoneCalls++;
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status CreateTrigger(const ZoneTriggerControlStruct & trigger) override
    {
        mPersistedTriggers.push_back(trigger);
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status UpdateTrigger(const ZoneTriggerControlStruct & trigger) override
    {
        for (auto & existing : mPersistedTriggers)
        {
            if (existing.zoneID == trigger.zoneID)
            {
                existing = trigger;
                return Protocols::InteractionModel::Status::Success;
            }
        }

        return Protocols::InteractionModel::Status::NotFound;
    }

    Protocols::InteractionModel::Status RemoveTrigger(uint16_t zoneId) override
    {
        mPersistedTriggers.erase(
            std::remove_if(mPersistedTriggers.begin(), mPersistedTriggers.end(),
                           [zoneId](const ZoneTriggerControlStruct & trigger) { return trigger.zoneID == zoneId; }),
            mPersistedTriggers.end());
        return Protocols::InteractionModel::Status::Success;
    }

    void OnAttributeChanged(AttributeId attributeId) override { mChangedAttributes.push_back(attributeId); }

    CHIP_ERROR PersistentAttributesLoadedCallback() override
    {
        mPersistentAttributesLoadedCalls++;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR LoadZones(std::vector<ZoneInformationStorage> & zones) override
    {
        zones = mPersistedZones;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR LoadTriggers(std::vector<ZoneTriggerControlStruct> & triggers) override
    {
        triggers = mPersistedTriggers;
        return CHIP_NO_ERROR;
    }

    bool IsValidAnalysisZone(const uint16_t zoneId) override { return true; }

    std::vector<ZoneInformationStorage> mPersistedZones;
    std::vector<ZoneTriggerControlStruct> mPersistedTriggers;
    std::vector<AttributeId> mChangedAttributes;
    uint16_t mNextZoneId                      = 1;
    unsigned mCreateZoneCalls                 = 0;
    unsigned mUpdateZoneCalls                 = 0;
    unsigned mRemoveZoneCalls                 = 0;
    unsigned mPersistentAttributesLoadedCalls = 0;
};

class TestZoneManagementCluster : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    ZoneManagementCluster CreateCluster(BitFlags<Feature> features)
    {
        return CreateCluster(features, kMaxUserDefinedZones, kMaxZones, kSensitivityMax, kTwoDMaxPoint);
    }

    ZoneManagementCluster CreateCluster(BitFlags<Feature> features, uint8_t maxUserDefinedZones, uint8_t maxZones,
                                        uint8_t sensitivityMax, const TwoDCartesianVertexStruct & twoDCartesianMax)
    {
        return ZoneManagementCluster(ZoneManagementCluster::Context{
            .delegate   = mDelegate,
            .endpointId = kTestEndpointId,
            .features   = features,
            .config     = {
                .maxUserDefinedZones = maxUserDefinedZones,
                .maxZones            = maxZones,
                .sensitivityMax      = sensitivityMax,
                .twoDCartesianMax    = twoDCartesianMax,
            },
        });
    }

    static ConcreteAttributePath SensitivityPath()
    {
        return ConcreteAttributePath(kTestEndpointId, ZoneManagement::Id, Attributes::Sensitivity::Id);
    }

    static std::array<TwoDCartesianVertexStruct, 3> MakeTriangle(uint16_t offset)
    {
        return { TwoDCartesianVertexStruct{ static_cast<uint16_t>(10 + offset), static_cast<uint16_t>(20 + offset) },
                 TwoDCartesianVertexStruct{ static_cast<uint16_t>(30 + offset), static_cast<uint16_t>(20 + offset) },
                 TwoDCartesianVertexStruct{ static_cast<uint16_t>(20 + offset), static_cast<uint16_t>(40 + offset) } };
    }

    static Commands::CreateTwoDCartesianZone::Type MakeCreateZoneRequest(const char * name,
                                                                         const std::array<TwoDCartesianVertexStruct, 3> & vertices)
    {
        Commands::CreateTwoDCartesianZone::Type request;
        request.zone.name     = CharSpan::fromCharString(name);
        request.zone.use      = ZoneUseEnum::kMotion;
        request.zone.vertices = DataModel::List<const TwoDCartesianVertexStruct>(vertices.data(), vertices.size());
        request.zone.color    = NullOptional;
        return request;
    }

    MockDelegate mDelegate;
};

TEST_F(TestZoneManagementCluster, AcceptedCommandsAppendElementsGrowsBeyondInitialCapacity)
{
    ZoneManagementCluster cluster = CreateCluster(BitFlags<Feature>(Feature::kUserDefined, Feature::kTwoDimensionalCartesianZone));
    ConcreteClusterPath path(kTestEndpointId, ZoneManagement::Id);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
    ASSERT_EQ(builder.EnsureAppendCapacity(1), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.AcceptedCommands(path, builder), CHIP_NO_ERROR);

    static constexpr DataModel::AcceptedCommandEntry kExpected[] = {
        Commands::CreateTwoDCartesianZone::kMetadataEntry,
        Commands::UpdateTwoDCartesianZone::kMetadataEntry,
        Commands::RemoveZone::kMetadataEntry,
        Commands::CreateOrUpdateTrigger::kMetadataEntry,
        Commands::RemoveTrigger::kMetadataEntry,
    };

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expectedBuilder;
    ASSERT_EQ(expectedBuilder.ReferenceExisting(kExpected), CHIP_NO_ERROR);
    ASSERT_TRUE(EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster, { kExpected[0], kExpected[1], kExpected[2], kExpected[3], kExpected[4] }));
}

TEST_F(TestZoneManagementCluster, AcceptedCommandsFollowFeatureGates)
{
    ZoneManagementCluster userDefinedOnly = CreateCluster(BitFlags<Feature>(Feature::kUserDefined));
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(userDefinedOnly,
                                              {
                                                  Commands::RemoveZone::kMetadataEntry,
                                                  Commands::CreateOrUpdateTrigger::kMetadataEntry,
                                                  Commands::RemoveTrigger::kMetadataEntry,
                                              }));

    ZoneManagementCluster noOptionalFeatures = CreateCluster(BitFlags<Feature>());
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(noOptionalFeatures,
                                              {
                                                  Commands::CreateOrUpdateTrigger::kMetadataEntry,
                                                  Commands::RemoveTrigger::kMetadataEntry,
                                              }));
}

TEST_F(TestZoneManagementCluster, AttributeListFollowsFeatureGates)
{
    ZoneManagementCluster cluster = CreateCluster(BitFlags<Feature>(Feature::kUserDefined, Feature::kTwoDimensionalCartesianZone));

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::MaxUserDefinedZones::kMetadataEntry,
                                            Attributes::MaxZones::kMetadataEntry,
                                            Attributes::Zones::kMetadataEntry,
                                            Attributes::Triggers::kMetadataEntry,
                                            Attributes::SensitivityMax::kMetadataEntry,
                                            Attributes::Sensitivity::kMetadataEntry,
                                            Attributes::TwoDCartesianMax::kMetadataEntry,
                                        }));
}

TEST_F(TestZoneManagementCluster, FeatureMapAttributeEncodesConfiguredFeatures)
{
    const BitFlags<Feature> features(Feature::kTwoDimensionalCartesianZone, Feature::kUserDefined, Feature::kFocusZones);
    ZoneManagementCluster cluster = CreateCluster(features);
    ClusterTester tester(cluster);

    uint32_t featureMap = 0;
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap), Protocols::InteractionModel::Status::Success);
    ASSERT_EQ(featureMap, features.Raw());
}

TEST_F(TestZoneManagementCluster, SensitivityWritePersistsAndReloadsOnStartup)
{
    auto cluster = CreateCluster(BitFlags<Feature>());
    Testing::TestServerClusterContext context;

    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    ASSERT_EQ(tester.WriteAttribute(Attributes::Sensitivity::Id, static_cast<uint8_t>(3)),
              Protocols::InteractionModel::Status::Success);
    ASSERT_EQ(cluster.GetSensitivity(), 3);

    uint8_t persistedSensitivity = 0;
    AttributePersistence persistence(context.Get().attributeStorage);
    ASSERT_TRUE(persistence.LoadNativeEndianValue(SensitivityPath(), persistedSensitivity, static_cast<uint8_t>(0)));
    ASSERT_EQ(persistedSensitivity, 3);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);

    auto reloadedCluster = CreateCluster(BitFlags<Feature>());
    ASSERT_EQ(reloadedCluster.Startup(context.Get()), CHIP_NO_ERROR);
    ASSERT_EQ(reloadedCluster.GetSensitivity(), 3);
}

TEST_F(TestZoneManagementCluster, GeneratedCommandsAppendElementsGrowsBeyondInitialCapacity)
{
    ZoneManagementCluster cluster = CreateCluster(BitFlags<Feature>(Feature::kUserDefined, Feature::kTwoDimensionalCartesianZone));
    ConcreteClusterPath path(kTestEndpointId, ZoneManagement::Id);

    ReadOnlyBufferBuilder<CommandId> builder;
    ASSERT_EQ(builder.EnsureAppendCapacity(0), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.GeneratedCommands(path, builder), CHIP_NO_ERROR);

    static constexpr CommandId kExpected[] = {
        Commands::CreateTwoDCartesianZoneResponse::Id,
    };

    ReadOnlyBufferBuilder<CommandId> expectedBuilder;
    ASSERT_EQ(expectedBuilder.ReferenceExisting(kExpected), CHIP_NO_ERROR);
    ASSERT_TRUE(EqualGeneratedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster, { kExpected[0] }));
}

TEST_F(TestZoneManagementCluster, GeneratedCommandsEmptyWhenCreateIsNotSupported)
{
    ZoneManagementCluster userDefinedOnly = CreateCluster(BitFlags<Feature>(Feature::kUserDefined));
    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(userDefinedOnly, {}));

    ZoneManagementCluster twoDOnly = CreateCluster(BitFlags<Feature>(Feature::kTwoDimensionalCartesianZone));
    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(twoDOnly, {}));
}

TEST_F(TestZoneManagementCluster, StartupLoadsPersistedZonesAndTriggersFromDelegate)
{
    const auto vertices = MakeTriangle(0);
    TwoDCartesianZoneStorage zone;
    zone.Set("Entry"_span, ZoneUseEnum::kMotion, std::vector<TwoDCartesianVertexStruct>(vertices.begin(), vertices.end()),
             NullOptional);

    ZoneInformationStorage zoneInfo;
    zoneInfo.Set(55, ZoneTypeEnum::kTwoDCARTZone, ZoneSourceEnum::kMfg, MakeOptional(zone));
    mDelegate.mPersistedZones.push_back(zoneInfo);

    ZoneTriggerControlStruct trigger;
    trigger.zoneID               = 55;
    trigger.initialDuration      = 10;
    trigger.augmentationDuration = 5;
    trigger.maxDuration          = 10;
    trigger.blindDuration        = 0;
    trigger.sensitivity          = NullOptional;
    mDelegate.mPersistedTriggers.push_back(trigger);

    auto cluster = CreateCluster(BitFlags<Feature>());
    Testing::TestServerClusterContext context;

    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ASSERT_EQ(mDelegate.mPersistentAttributesLoadedCalls, 1u);
    ASSERT_EQ(cluster.GetZones().size(), 1u);
    ASSERT_EQ(cluster.GetTriggers().size(), 1u);
    ASSERT_TRUE(cluster.GetTriggerForZone(55).HasValue());
}

TEST_F(TestZoneManagementCluster, InvokeCreateUpdateAndRemoveZoneCommands)
{
    auto cluster = CreateCluster(BitFlags<Feature>(Feature::kUserDefined, Feature::kTwoDimensionalCartesianZone));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    const auto createVertices = MakeTriangle(0);
    auto createResult         = tester.Invoke(MakeCreateZoneRequest("Zone One", createVertices));
    ASSERT_TRUE(createResult.IsSuccess());
    if (!createResult.response.has_value())
    {
        FAIL() << "Expected createResult.response to have a value";
        return;
    }
    ASSERT_EQ(createResult.response->zoneID, 1);
    ASSERT_EQ(mDelegate.mCreateZoneCalls, 1u);
    ASSERT_EQ(cluster.GetZones().size(), 1u);
    ASSERT_TRUE(cluster.GetZones().front().twoDCartZoneStorage.HasValue());
    ASSERT_TRUE(cluster.GetZones().front().twoDCartZoneStorage.Value().name.data_equal("Zone One"_span));

    const auto updateVertices = MakeTriangle(100);
    Commands::UpdateTwoDCartesianZone::Type updateRequest;
    updateRequest.zoneID        = 1;
    updateRequest.zone.name     = "Zone Two"_span;
    updateRequest.zone.use      = ZoneUseEnum::kMotion;
    updateRequest.zone.vertices = DataModel::List<const TwoDCartesianVertexStruct>(updateVertices.data(), updateVertices.size());
    updateRequest.zone.color    = NullOptional;

    auto updateResult = tester.Invoke(updateRequest);
    ASSERT_TRUE(updateResult.IsSuccess());
    ASSERT_EQ(mDelegate.mUpdateZoneCalls, 1u);
    ASSERT_EQ(cluster.GetZones().size(), 1u);
    ASSERT_TRUE(cluster.GetZones().front().twoDCartZoneStorage.Value().name.data_equal("Zone Two"_span));

    Commands::RemoveZone::Type removeRequest;
    removeRequest.zoneID = 1;

    auto removeResult = tester.Invoke(removeRequest);
    ASSERT_TRUE(removeResult.IsSuccess());
    ASSERT_EQ(mDelegate.mRemoveZoneCalls, 1u);
    ASSERT_TRUE(cluster.GetZones().empty());
    ASSERT_TRUE(mDelegate.mPersistedZones.empty());
}

TEST_F(TestZoneManagementCluster, StartupFailsForInvalidConfiguration)
{
    Testing::TestServerClusterContext context;

    auto invalidSensitivity = CreateCluster(BitFlags<Feature>(), kMaxUserDefinedZones, kMaxZones, 1, kTwoDMaxPoint);
    ASSERT_EQ(invalidSensitivity.Startup(context.Get()), CHIP_ERROR_INVALID_ARGUMENT);

    auto invalidUserDefined = CreateCluster(BitFlags<Feature>(Feature::kUserDefined), 4, kMaxZones, kSensitivityMax, kTwoDMaxPoint);
    ASSERT_EQ(invalidUserDefined.Startup(context.Get()), CHIP_ERROR_INVALID_ARGUMENT);

    auto remoteZonesWithoutUserDefined =
        CreateCluster(BitFlags<Feature>(Feature::kRemoteZones), kMaxUserDefinedZones, kMaxZones, kSensitivityMax, kTwoDMaxPoint);
    ASSERT_EQ(remoteZonesWithoutUserDefined.Startup(context.Get()), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestZoneManagementCluster, RemoteZonesRejectedWhenFeatureDisabled)
{
    auto cluster = CreateCluster(BitFlags<Feature>(Feature::kUserDefined, Feature::kTwoDimensionalCartesianZone));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    const auto vertices = MakeTriangle(0);
    auto createWithNode = MakeCreateZoneRequest("Zone One", vertices);
    createWithNode.nodeID.SetValue(DataModel::MakeNullable<NodeId>(0x1234));
    auto createResult = tester.Invoke(createWithNode);
    ASSERT_EQ(createResult.GetStatusCode(),
              Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    auto createLocal = MakeCreateZoneRequest("Zone One", vertices);
    auto okCreate    = tester.Invoke(createLocal);
    ASSERT_TRUE(okCreate.IsSuccess());

    const auto updateVertices = MakeTriangle(50);
    Commands::UpdateTwoDCartesianZone::Type updateWithNode;
    updateWithNode.zoneID        = 1;
    updateWithNode.zone.name     = "Zone Two"_span;
    updateWithNode.zone.use      = ZoneUseEnum::kMotion;
    updateWithNode.zone.vertices = DataModel::List<const TwoDCartesianVertexStruct>(updateVertices.data(), updateVertices.size());
    updateWithNode.nodeID.SetValue(DataModel::MakeNullable<NodeId>(0x1234));
    auto updateResult = tester.Invoke(updateWithNode);
    ASSERT_EQ(updateResult.GetStatusCode(),
              Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));

    // Even when zoneID does not exist, update with remote fields when kRemoteZones is disabled must return InvalidCommand
    updateWithNode.zoneID = 999;
    ASSERT_EQ(tester.Invoke(updateWithNode).GetStatusCode(),
              Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::InvalidCommand));
}

TEST_F(TestZoneManagementCluster, RemoteZonesCreateUpdateValidationAndEventFields)
{
    auto cluster =
        CreateCluster(BitFlags<Feature>(Feature::kUserDefined, Feature::kTwoDimensionalCartesianZone, Feature::kRemoteZones));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    const auto vertices = MakeTriangle(0);

    // Invalid NodeID (kUndefinedNodeId) -> ConstraintError
    auto invalidNodeReq = MakeCreateZoneRequest("Invalid Node", vertices);
    invalidNodeReq.nodeID.SetValue(DataModel::MakeNullable<NodeId>(kUndefinedNodeId));
    ASSERT_EQ(tester.Invoke(invalidNodeReq).GetStatusCode(),
              Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::ConstraintError));

    // Non-null EndpointID with null NodeID -> ConstraintError
    auto endpointWithoutNodeReq = MakeCreateZoneRequest("No Node", vertices);
    endpointWithoutNodeReq.nodeID.SetValue(DataModel::NullNullable);
    endpointWithoutNodeReq.endpointID.SetValue(DataModel::MakeNullable<EndpointId>(2));
    ASSERT_EQ(tester.Invoke(endpointWithoutNodeReq).GetStatusCode(),
              Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::ConstraintError));

    // Invalid EndpointID (kInvalidEndpointId) -> ConstraintError
    auto invalidEndpointReq = MakeCreateZoneRequest("Bad Endpoint", vertices);
    invalidEndpointReq.nodeID.SetValue(DataModel::MakeNullable<NodeId>(0x1234));
    invalidEndpointReq.endpointID.SetValue(DataModel::MakeNullable<EndpointId>(kInvalidEndpointId));
    ASSERT_EQ(tester.Invoke(invalidEndpointReq).GetStatusCode(),
              Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::ConstraintError));

    // Valid remote zone creation with NodeID and EndpointID
    auto createRemoteReq = MakeCreateZoneRequest("Remote Zone", vertices);
    createRemoteReq.nodeID.SetValue(DataModel::MakeNullable<NodeId>(0x1234));
    createRemoteReq.endpointID.SetValue(DataModel::MakeNullable<EndpointId>(2));
    auto createResult = tester.Invoke(createRemoteReq);
    ASSERT_TRUE(createResult.IsSuccess());
    ASSERT_EQ(cluster.GetZones().size(), 1u);
    ASSERT_TRUE(cluster.GetZones().front().nodeID.HasValue());
    ASSERT_EQ(cluster.GetZones().front().nodeID.Value(), static_cast<NodeId>(0x1234));
    ASSERT_TRUE(cluster.GetZones().front().endpointID.HasValue());
    ASSERT_EQ(cluster.GetZones().front().endpointID.Value(), static_cast<EndpointId>(2));

    // Event generation for remote zone populates nodeID and endpointID
    ASSERT_EQ(cluster.GenerateZoneTriggeredEvent(1, ZoneEventTriggeredReasonEnum::kMotion),
              Protocols::InteractionModel::Status::Success);
    auto triggeredEvent = tester.GetNextGeneratedEvent();
    if (!triggeredEvent.has_value())
    {
        FAIL() << "Expected triggeredEvent to have a value";
        return;
    }
    Events::ZoneTriggered::DecodableType triggeredData;
    ASSERT_EQ(triggeredEvent->GetEventData(triggeredData), CHIP_NO_ERROR);
    ASSERT_EQ(triggeredData.zone, 1);
    ASSERT_TRUE(triggeredData.nodeID.HasValue());
    ASSERT_EQ(triggeredData.nodeID.Value(), static_cast<NodeId>(0x1234));
    ASSERT_TRUE(triggeredData.endpointID.HasValue());
    ASSERT_EQ(triggeredData.endpointID.Value(), static_cast<EndpointId>(2));

    ASSERT_EQ(cluster.GenerateZoneStoppedEvent(1, ZoneEventStoppedReasonEnum::kActionStopped),
              Protocols::InteractionModel::Status::Success);
    auto stoppedEvent = tester.GetNextGeneratedEvent();
    if (!stoppedEvent.has_value())
    {
        FAIL() << "Expected stoppedEvent to have a value";
        return;
    }
    Events::ZoneStopped::DecodableType stoppedData;
    ASSERT_EQ(stoppedEvent->GetEventData(stoppedData), CHIP_NO_ERROR);
    ASSERT_EQ(stoppedData.zone, 1);
    ASSERT_TRUE(stoppedData.nodeID.HasValue());
    ASSERT_EQ(stoppedData.nodeID.Value(), static_cast<NodeId>(0x1234));
    ASSERT_TRUE(stoppedData.endpointID.HasValue());
    ASSERT_EQ(stoppedData.endpointID.Value(), static_cast<EndpointId>(2));

    // Updating geometry/name while omitting nodeID and endpointID preserves existing remote zone identifiers
    const auto updatePreserveVertices = MakeTriangle(25);
    Commands::UpdateTwoDCartesianZone::Type updatePreserveReq;
    updatePreserveReq.zoneID    = 1;
    updatePreserveReq.zone.name = "Preserved Remote"_span;
    updatePreserveReq.zone.use  = ZoneUseEnum::kMotion;
    updatePreserveReq.zone.vertices =
        DataModel::List<const TwoDCartesianVertexStruct>(updatePreserveVertices.data(), updatePreserveVertices.size());
    ASSERT_TRUE(tester.Invoke(updatePreserveReq).IsSuccess());
    ASSERT_TRUE(cluster.GetZones().front().nodeID.HasValue());
    ASSERT_EQ(cluster.GetZones().front().nodeID.Value(), static_cast<NodeId>(0x1234));
    ASSERT_TRUE(cluster.GetZones().front().endpointID.HasValue());
    ASSERT_EQ(cluster.GetZones().front().endpointID.Value(), static_cast<EndpointId>(2));

    // Clearing nodeID while keeping non-null endpointID (either omitted or explicitly provided) -> ConstraintError
    const auto invalidUpdateVertices = MakeTriangle(35);
    Commands::UpdateTwoDCartesianZone::Type clearNodeOnlyReq;
    clearNodeOnlyReq.zoneID    = 1;
    clearNodeOnlyReq.zone.name = "Invalid Clear"_span;
    clearNodeOnlyReq.zone.use  = ZoneUseEnum::kMotion;
    clearNodeOnlyReq.zone.vertices =
        DataModel::List<const TwoDCartesianVertexStruct>(invalidUpdateVertices.data(), invalidUpdateVertices.size());
    clearNodeOnlyReq.nodeID.SetValue(DataModel::NullNullable);
    ASSERT_EQ(tester.Invoke(clearNodeOnlyReq).GetStatusCode(),
              Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::ConstraintError));

    // Update remote zone to a new NodeID and null EndpointID
    const auto updateVertices = MakeTriangle(50);
    Commands::UpdateTwoDCartesianZone::Type updateRemoteReq;
    updateRemoteReq.zoneID        = 1;
    updateRemoteReq.zone.name     = "Updated Remote"_span;
    updateRemoteReq.zone.use      = ZoneUseEnum::kMotion;
    updateRemoteReq.zone.vertices = DataModel::List<const TwoDCartesianVertexStruct>(updateVertices.data(), updateVertices.size());
    updateRemoteReq.nodeID.SetValue(DataModel::MakeNullable<NodeId>(0x5678));
    updateRemoteReq.endpointID.SetValue(DataModel::NullNullable);
    ASSERT_TRUE(tester.Invoke(updateRemoteReq).IsSuccess());
    ASSERT_TRUE(cluster.GetZones().front().nodeID.HasValue());
    ASSERT_EQ(cluster.GetZones().front().nodeID.Value(), static_cast<NodeId>(0x5678));
    ASSERT_FALSE(cluster.GetZones().front().endpointID.HasValue());

    // Shutdown and restart cluster to verify remote zone fields were persisted via Delegate
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    auto reloadedCluster =
        CreateCluster(BitFlags<Feature>(Feature::kUserDefined, Feature::kTwoDimensionalCartesianZone, Feature::kRemoteZones));
    ASSERT_EQ(reloadedCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    ASSERT_EQ(reloadedCluster.GetZones().size(), 1u);
    ASSERT_TRUE(reloadedCluster.GetZones().front().nodeID.HasValue());
    ASSERT_EQ(reloadedCluster.GetZones().front().nodeID.Value(), static_cast<NodeId>(0x5678));
    ASSERT_FALSE(reloadedCluster.GetZones().front().endpointID.HasValue());
}

TEST_F(TestZoneManagementCluster, EventGenerationProducesTriggeredAndStoppedEvents)
{
    auto cluster = CreateCluster(BitFlags<Feature>());
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_EQ(cluster.GenerateZoneTriggeredEvent(7, ZoneEventTriggeredReasonEnum::kMotion),
              Protocols::InteractionModel::Status::Success);
    auto triggeredEvent = tester.GetNextGeneratedEvent();
    if (!triggeredEvent.has_value())
    {
        FAIL() << "Expected triggeredEvent to have a value";
        return;
    }
    Events::ZoneTriggered::DecodableType triggeredData;
    ASSERT_EQ(triggeredEvent->GetEventData(triggeredData), CHIP_NO_ERROR);
    ASSERT_EQ(triggeredData.zone, 7);
    ASSERT_EQ(triggeredData.reason, ZoneEventTriggeredReasonEnum::kMotion);

    ASSERT_EQ(cluster.GenerateZoneStoppedEvent(7, ZoneEventStoppedReasonEnum::kActionStopped),
              Protocols::InteractionModel::Status::Success);
    auto stoppedEvent = tester.GetNextGeneratedEvent();
    if (!stoppedEvent.has_value())
    {
        FAIL() << "Expected stoppedEvent to have a value";
        return;
    }
    Events::ZoneStopped::DecodableType stoppedData;
    ASSERT_EQ(stoppedEvent->GetEventData(stoppedData), CHIP_NO_ERROR);
    ASSERT_EQ(stoppedData.zone, 7);
    ASSERT_EQ(stoppedData.reason, ZoneEventStoppedReasonEnum::kActionStopped);
}

} // namespace
