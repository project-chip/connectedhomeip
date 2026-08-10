/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <pw_unit_test/framework.h>
#include <ranges>

#include <app/CommandHandler.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/av-analysis-server/AvAnalysisCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model/Decode.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/AvAnalysis/Attributes.h>
#include <clusters/AvAnalysis/Commands.h>
#include <clusters/AvAnalysis/Enums.h>
#include <clusters/AvAnalysis/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>

#include "MockAvAnalysisDelegate.h"

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AvAnalysis;

using namespace Protocols::InteractionModel;
using namespace chip::Testing;

using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

static constexpr chip::EndpointId kTestEndpointId = 1;
static constexpr uint8_t kTestMaxZones            = 5;
static constexpr uint8_t kTestMaxAnalysisStreams  = 8;

// Test ambient contexts
// Define the list of semantic tags for the endpoint
const std::vector<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> testAmbientContexts = {
    { std::nullopt, static_cast<uint8_t>(0x49), static_cast<uint8_t>(0x0B),
      MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Object.Package"_span)) },
    { std::nullopt, static_cast<uint8_t>(0x4B), static_cast<uint8_t>(0x08),
      MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Activity.Delivery"_span)) },
    { std::nullopt, static_cast<uint8_t>(0x4B), static_cast<uint8_t>(0x09),
      MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Activity.Retrieval"_span)) }
};

// Minimal mock delegate for testing
struct TestRemoteAvAnalysisCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestRemoteAvAnalysisCluster() :
        mServer(kTestEndpointId, chip::BitFlags<Feature>(Feature::kRemoteContextDetection, Feature::kPerZoneContextDetection),
                testAmbientContexts, DataModel::MakeNullable(kTestMaxZones), kTestMaxAnalysisStreams),
        mClusterTester(mServer)
    {}

    // Records camera interactions started by the command handlers; tests complete them by
    // invoking the recorded Callback, simulating the camera's response.
    class FakeCameraClient : public AvAnalysisCameraClient
    {
    public:
        int mAllocationRequests   = 0;
        int mDeallocationRequests = 0;
        ScopedNodeId mLastCamera;
        uint16_t mLastStreamId   = 0;
        Callback * mLastCallback = nullptr;

        CHIP_ERROR RequestVideoStreamAllocation(const ScopedNodeId & aCameraNode, Callback & aCallback) override
        {
            mAllocationRequests++;
            mLastCamera   = aCameraNode;
            mLastCallback = &aCallback;
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR RequestVideoStreamDeallocation(const ScopedNodeId & aCameraNode, uint16_t aAnalysisStreamId,
                                                  Callback & aCallback) override
        {
            mDeallocationRequests++;
            mLastCamera   = aCameraNode;
            mLastStreamId = aAnalysisStreamId;
            mLastCallback = &aCallback;
            return CHIP_NO_ERROR;
        }
    };

    void SetUp() override
    {
        mServer.SetDelegate(&mMockDelegate);
        mServer.SetCameraClient(&mFakeCameraClient);
        EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
        EXPECT_EQ(mServer.Init(), CHIP_NO_ERROR);
    }

    void TearDown() override { mServer.Shutdown(ClusterShutdownType::kClusterShutdown); }

    // Sends EstablishAnalysisStream for the given camera node and completes the camera allocation
    // with the given status/stream id; returns the mock handler carrying response or status.
    void EstablishStream(Testing::MockCommandHandler & commandHandler, NodeId aCameraNodeId, Status aCameraStatus,
                         uint16_t aVideoStreamId)
    {
        ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
        Commands::EstablishAnalysisStream::DecodableType commandData;
        commandData.nodeID = aCameraNodeId;

        auto response = mServer.GetLogic().HandleEstablishAnalysisStream(commandHandler, path, commandData);
        ASSERT_FALSE(response.has_value()); // Response is pending on the camera interaction
        ASSERT_NE(mFakeCameraClient.mLastCallback, nullptr);
        mFakeCameraClient.mLastCallback->OnVideoStreamAllocated(aCameraStatus, aVideoStreamId);
    }

    MockAvAnalysisDelegate mMockDelegate;
    FakeCameraClient mFakeCameraClient;
    AvAnalysisCluster mServer;
    ClusterTester mClusterTester;
};

TEST_F(TestRemoteAvAnalysisCluster, TestAttributes)
{
    ASSERT_TRUE(IsAttributesListEqualTo(mServer,
                                        {
                                            Attributes::SupportedAmbientContexts::kMetadataEntry,
                                            Attributes::ActiveAmbientContextTriggers::kMetadataEntry,
                                            Attributes::MaxAnalysisStreamCount::kMetadataEntry,
                                            Attributes::CurrentAnalysisStreamCount::kMetadataEntry,
                                            Attributes::AnalysisStreams::kMetadataEntry,
                                            Attributes::TrackingEnabled::kMetadataEntry,
                                        }));
}

TEST_F(TestRemoteAvAnalysisCluster, TestCommands)
{
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(mServer,
                                              {
                                                  Commands::EnableContextTriggers::kMetadataEntry,
                                                  Commands::DisableContextTriggers::kMetadataEntry,
                                                  Commands::EstablishAnalysisStream::kMetadataEntry,
                                                  Commands::ActivateAnalysisStream::kMetadataEntry,
                                                  Commands::DeactivateAnalysisStream::kMetadataEntry,
                                                  Commands::RemoveAnalysisStream::kMetadataEntry,
                                              }));
}

TEST_F(TestRemoteAvAnalysisCluster, ReadAllAttributesWithClusterTesterTest)
{
    Attributes::SupportedAmbientContexts::TypeInfo::DecodableType aSupportedAmbientContexts;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::SupportedAmbientContexts::Id, aSupportedAmbientContexts), CHIP_NO_ERROR);

    // Verify that the entries in the DecodableList match the entries used in construction of the instance by
    // creating a vactor of the values then comparing the two vectors
    std::vector<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> readContexts;
    auto aContextIterator = aSupportedAmbientContexts.begin();
    while (aContextIterator.Next())
    {
        readContexts.push_back(aContextIterator.GetValue());
    }

    // No == exists for the Struct, and creating one fails due to the Struct structure, check value by value
    bool are_equal =
        std::equal(testAmbientContexts.begin(), testAmbientContexts.end(), readContexts.begin(), readContexts.end(),
                   [](const auto & p1, const auto & p2) { return p1.namespaceID == p2.namespaceID && p1.tag == p2.tag; });
    ASSERT_TRUE(are_equal);

    // On startup there should be no active triggers
    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType aActiveContextTriggers;
    size_t triggersSize;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, aActiveContextTriggers), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED aActiveContextTriggers.ComputeSize(&triggersSize);
    ASSERT_EQ(triggersSize, static_cast<size_t>(0));

    uint8_t aMaxAnalysisStreamCount = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MaxAnalysisStreamCount::Id, aMaxAnalysisStreamCount), CHIP_NO_ERROR);
    ASSERT_EQ(kTestMaxAnalysisStreams, aMaxAnalysisStreamCount);

    uint8_t aCurrentAnalysisStreamCount = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, aCurrentAnalysisStreamCount), CHIP_NO_ERROR);
    ASSERT_EQ(0, aCurrentAnalysisStreamCount);

    // On startup there should be no analysis streams
    Attributes::AnalysisStreams::TypeInfo::DecodableType aAnalysisStreams;
    size_t streamsSize;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, aAnalysisStreams), CHIP_NO_ERROR);
    TEMPORARY_RETURN_IGNORED aAnalysisStreams.ComputeSize(&streamsSize);
    ASSERT_EQ(streamsSize, static_cast<size_t>(0));

    bool trackingEnabled = false;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::TrackingEnabled::Id, trackingEnabled), CHIP_NO_ERROR);
    ASSERT_FALSE(trackingEnabled);
}

TEST_F(TestRemoteAvAnalysisCluster, ExecuteEnableContextTriggersCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;

    auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, kCommandPath, commandData);

    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        // Fail the test case
        FAIL();
    }
}

TEST_F(TestRemoteAvAnalysisCluster, ExecuteDisableContextTriggersCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::DisableContextTriggers::Id };
    Commands::DisableContextTriggers::DecodableType commandData;

    auto response = mServer.GetLogic().HandleDisableContextTriggers(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        FAIL();
    }
}

TEST_F(TestRemoteAvAnalysisCluster, EstablishAnalysisStreamSuccess)
{
    constexpr NodeId kCameraNodeId = 0x1234;
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);

    EstablishStream(commandHandler, kCameraNodeId, Status::Success, 42);

    // The camera was asked to allocate, scoped to the invoking client's fabric
    ASSERT_EQ(mFakeCameraClient.mAllocationRequests, 1);
    ASSERT_EQ(mFakeCameraClient.mLastCamera, ScopedNodeId(kCameraNodeId, 1));

    // EstablishAnalysisStreamResponse carries the camera-assigned stream id
    ASSERT_TRUE(commandHandler.HasResponse());
    ASSERT_EQ(commandHandler.GetResponseCommandId(), Commands::EstablishAnalysisStreamResponse::Id);
    Commands::EstablishAnalysisStreamResponse::DecodableType response;
    ASSERT_EQ(commandHandler.DecodeResponse(response), CHIP_NO_ERROR);
    ASSERT_EQ(response.analysisStreamID, 42);

    // The stream is tracked in PendingInitiation
    uint8_t currentCount = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 1);

    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamID, 42);
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kPendingInitiation);
}

TEST_F(TestRemoteAvAnalysisCluster, EstablishAnalysisStreamPropagatesCameraFailure)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);

    EstablishStream(commandHandler, 0x1234, Status::ResourceExhausted, 0);

    ASSERT_FALSE(commandHandler.HasResponse());
    ASSERT_TRUE(commandHandler.HasStatus());
    ASSERT_EQ(commandHandler.GetLastStatus().status.GetStatus(), Status::ResourceExhausted);

    uint8_t currentCount = 0xFF;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 0);
}

TEST_F(TestRemoteAvAnalysisCluster, EstablishAnalysisStreamBusyWhileAllocationPending)
{
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
    Commands::EstablishAnalysisStream::DecodableType commandData;
    commandData.nodeID = 0x1234;

    Testing::MockCommandHandler firstHandler;
    firstHandler.SetFabricIndex(1);
    auto firstResponse = mServer.GetLogic().HandleEstablishAnalysisStream(firstHandler, path, commandData);
    ASSERT_FALSE(firstResponse.has_value()); // Pending on the camera

    // A second camera-bound command while the first is pending is answered with Busy
    Testing::MockCommandHandler secondHandler;
    secondHandler.SetFabricIndex(1);
    auto secondResponse = mServer.GetLogic().HandleEstablishAnalysisStream(secondHandler, path, commandData);
    if (secondResponse.has_value())
    {
        ASSERT_TRUE(secondResponse->GetStatusCode() == Protocols::InteractionModel::ClusterStatusCode(Status::Busy));
    }
    else
    {
        FAIL();
    }
    ASSERT_EQ(mFakeCameraClient.mAllocationRequests, 1);

    // Completing the first command frees the pending slot
    mFakeCameraClient.mLastCallback->OnVideoStreamAllocated(Status::Success, 7);
    ASSERT_TRUE(firstHandler.HasResponse());
}

TEST_F(TestRemoteAvAnalysisCluster, EstablishAnalysisStreamExhaustsAtCapacity)
{
    // Fill the table to MaxAnalysisStreamCount via established streams
    for (uint16_t id = 1; id <= kTestMaxAnalysisStreams; id++)
    {
        Testing::MockCommandHandler commandHandler;
        commandHandler.SetFabricIndex(1);
        EstablishStream(commandHandler, 0x1234, Status::Success, id);
        ASSERT_TRUE(commandHandler.HasResponse());
    }

    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
    Commands::EstablishAnalysisStream::DecodableType commandData;
    commandData.nodeID = 0x1234;
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);

    auto response = mServer.GetLogic().HandleEstablishAnalysisStream(commandHandler, path, commandData);
    if (response.has_value())
    {
        ASSERT_TRUE(response->GetStatusCode() == Protocols::InteractionModel::ClusterStatusCode(Status::ResourceExhausted));
    }
    else
    {
        FAIL();
    }
    ASSERT_EQ(mFakeCameraClient.mAllocationRequests, kTestMaxAnalysisStreams);
}

TEST_F(TestRemoteAvAnalysisCluster, EstablishAnalysisStreamWithoutCameraClientFails)
{
    mServer.SetCameraClient(nullptr);

    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
    Commands::EstablishAnalysisStream::DecodableType commandData;
    commandData.nodeID = 0x1234;
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);

    auto response = mServer.GetLogic().HandleEstablishAnalysisStream(commandHandler, path, commandData);
    if (response.has_value())
    {
        ASSERT_FALSE(response->IsSuccess());
    }
    else
    {
        FAIL();
    }
}

TEST_F(TestRemoteAvAnalysisCluster, ExecuteActivateAnalysisStreamCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
    Commands::ActivateAnalysisStream::DecodableType commandData;

    auto response = mServer.GetLogic().HandleActivateAnalysisStream(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        FAIL();
    }
}

TEST_F(TestRemoteAvAnalysisCluster, ExecuteDeactivateAnalysisStreamCommandTest)
{
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType commandData;

    auto response = mServer.GetLogic().HandleDeactivateAnalysisStream(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_TRUE(response.value().IsSuccess());
    }
    else
    {
        FAIL();
    }
}

TEST_F(TestRemoteAvAnalysisCluster, RemoveAnalysisStreamSuccess)
{
    constexpr NodeId kCameraNodeId = 0x1234;
    Testing::MockCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, kCameraNodeId, Status::Success, 42);

    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 42;
    Testing::MockCommandHandler removeHandler;
    removeHandler.SetFabricIndex(1);

    auto response = mServer.GetLogic().HandleRemoveAnalysisStream(removeHandler, path, commandData);
    ASSERT_FALSE(response.has_value()); // Pending on the camera

    // The deallocation goes to the camera the stream was established on
    ASSERT_EQ(mFakeCameraClient.mDeallocationRequests, 1);
    ASSERT_EQ(mFakeCameraClient.mLastCamera, ScopedNodeId(kCameraNodeId, 1));
    ASSERT_EQ(mFakeCameraClient.mLastStreamId, 42);

    mFakeCameraClient.mLastCallback->OnVideoStreamDeallocated(Status::Success, 42);

    ASSERT_TRUE(removeHandler.HasStatus());
    ASSERT_EQ(removeHandler.GetLastStatus().status.GetStatus(), Status::Success);

    // The stream is gone
    uint8_t currentCount = 0xFF;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 0);
}

TEST_F(TestRemoteAvAnalysisCluster, RemoveAnalysisStreamUnknownIdIsNotFound)
{
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 77; // never established
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);

    auto response = mServer.GetLogic().HandleRemoveAnalysisStream(commandHandler, path, commandData);
    if (response.has_value())
    {
        ASSERT_TRUE(response->GetStatusCode() == Protocols::InteractionModel::ClusterStatusCode(Status::NotFound));
    }
    else
    {
        FAIL();
    }
    ASSERT_EQ(mFakeCameraClient.mDeallocationRequests, 0);
}

TEST_F(TestRemoteAvAnalysisCluster, RemoveAnalysisStreamPropagatesCameraFailure)
{
    Testing::MockCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 42;
    Testing::MockCommandHandler removeHandler;
    removeHandler.SetFabricIndex(1);

    auto response = mServer.GetLogic().HandleRemoveAnalysisStream(removeHandler, path, commandData);
    ASSERT_FALSE(response.has_value());

    // The camera's status is returned and the stream entry is kept
    mFakeCameraClient.mLastCallback->OnVideoStreamDeallocated(Status::Failure, 42);

    ASSERT_EQ(removeHandler.GetLastStatus().status.GetStatus(), Status::Failure);

    uint8_t currentCount = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 1);
}

TEST_F(TestRemoteAvAnalysisCluster, AnalysisStreamsPersistAcrossRestart)
{
    constexpr NodeId kCameraNodeId = 0x1234;
    Testing::MockCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, kCameraNodeId, Status::Success, 42);

    // Restart the server against the same storage; AnalysisStreams has quality N
    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);
    EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint8_t currentCount = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 1);

    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamID, 42);
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kPendingInitiation);

    // The restored stream is still bound to its camera: removing it deallocates on that camera
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 42;
    Testing::MockCommandHandler removeHandler;
    removeHandler.SetFabricIndex(1);
    auto response = mServer.GetLogic().HandleRemoveAnalysisStream(removeHandler, path, commandData);
    ASSERT_FALSE(response.has_value());
    ASSERT_EQ(mFakeCameraClient.mLastCamera, ScopedNodeId(kCameraNodeId, 1));

    mFakeCameraClient.mLastCallback->OnVideoStreamDeallocated(Status::Success, 42);
    ASSERT_TRUE(removeHandler.HasStatus());
}

TEST_F(TestRemoteAvAnalysisCluster, ExecuteTrackingEnabledPersistenceTest)
{
    // Defaults to false, write true
    bool trackingEnabled = false;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::TrackingEnabled::Id, trackingEnabled), CHIP_NO_ERROR);
    ASSERT_FALSE(trackingEnabled);

    ASSERT_TRUE(mClusterTester.WriteAttribute(Attributes::TrackingEnabled::Id, true).IsSuccess());

    // Restart the server, the value has quality N and must survive
    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);
    EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::TrackingEnabled::Id, trackingEnabled), CHIP_NO_ERROR);
    ASSERT_TRUE(trackingEnabled);
}

TEST_F(TestRemoteAvAnalysisCluster, AnalysisStreamTableEncodeDecodeTest)
{
    AnalysisStreamTable table;
    ASSERT_EQ(table.Init(3), CHIP_NO_ERROR);

    // Ids come from the camera's VideoStreamAllocate; duplicates and overflow are rejected
    ASSERT_NE(table.Add(10), nullptr);
    ASSERT_NE(table.Add(20), nullptr);
    ASSERT_EQ(table.Add(10), nullptr); // duplicate id
    ASSERT_NE(table.Add(30), nullptr);
    ASSERT_EQ(table.Add(40), nullptr); // table full
    ASSERT_EQ(table.Count(), 3);

    // Record the camera association on every entry; give one an active session so decode's state
    // reset is observable
    const ScopedNodeId kCamera(0xABCD, 1);
    for (auto & entry : table)
    {
        entry.cameraNode = kCamera;
    }
    AnalysisStreamEntry * active = table.Find(20);
    ASSERT_NE(active, nullptr);
    active->state            = AnalysisStreamStateEnum::kWebRTCActive;
    active->webRTCEndpointID = DataModel::MakeNullable<EndpointId>(2);

    uint8_t buffer[AnalysisStreamTable::kEntrySerializedSize * 3 + AnalysisStreamTable::kArraySerializedOverhead];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));
    ASSERT_EQ(table.Encode(writer), CHIP_NO_ERROR);

    AnalysisStreamTable restored;
    ASSERT_EQ(restored.Init(3), CHIP_NO_ERROR);
    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ASSERT_EQ(restored.Decode(reader), CHIP_NO_ERROR);

    // All ids survive; sessions do not survive a reboot, so state collapses to PendingInitiation
    ASSERT_EQ(restored.Count(), 3);
    for (uint16_t id : { uint16_t(10), uint16_t(20), uint16_t(30) })
    {
        AnalysisStreamEntry * entry = restored.Find(id);
        ASSERT_NE(entry, nullptr);
        ASSERT_EQ(entry->state, AnalysisStreamStateEnum::kPendingInitiation);
        ASSERT_TRUE(entry->webRTCEndpointID.IsNull());
        ASSERT_TRUE(entry->pushAVEndpointID.IsNull());
        ASSERT_EQ(entry->cameraNode, kCamera); // The camera association must survive reboot
    }
}
} // namespace
