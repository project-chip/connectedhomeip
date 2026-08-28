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

        CHIP_ERROR RequestVideoStreamDeallocation(const ScopedNodeId & aCameraNode, uint16_t aVideoStreamId,
                                                  Callback & aCallback) override
        {
            mDeallocationRequests++;
            mLastCamera   = aCameraNode;
            mLastStreamId = aVideoStreamId;
            mLastCallback = &aCallback;
            return CHIP_NO_ERROR;
        }

        void Cancel() override
        {
            mCancelCount++;
            mLastCallback = nullptr;
        }

        int mCancelCount = 0;
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
    // creating a vector of the values then comparing the two vectors
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

    ASSERT_TRUE(commandHandler.HasResponse());
    ASSERT_EQ(commandHandler.GetResponseCommandId(), Commands::EstablishAnalysisStreamResponse::Id);
    Commands::EstablishAnalysisStreamResponse::DecodableType response;
    ASSERT_EQ(commandHandler.DecodeResponse(response), CHIP_NO_ERROR);
    ASSERT_EQ(response.analysisStreamID, 0);

    // The stream is tracked in PendingInitiation
    uint8_t currentCount = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 1);

    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamID, 0);
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

    commandData.analysisStreamID = 77;
    auto response                = mServer.GetLogic().HandleActivateAnalysisStream(commandHandler, kCommandPath, commandData);
    if (response.has_value())
    {
        ASSERT_TRUE(response->GetStatusCode() == Protocols::InteractionModel::ClusterStatusCode(Status::NotFound));
    }
    else
    {
        FAIL();
    }

    // TODO: activation is not implemented yet; no stream can leave PendingInitiation,
    // so the placeholder responds INVALID_IN_STATE for a known stream.
    Testing::MockCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    commandData.analysisStreamID = 0;
    response                     = mServer.GetLogic().HandleActivateAnalysisStream(commandHandler, kCommandPath, commandData);
    if (response.has_value())
    {
        ASSERT_TRUE(response->GetStatusCode() == Protocols::InteractionModel::ClusterStatusCode(Status::InvalidInState));
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

    // Spec 11.9.8.6.2: an unknown AnalysisStreamID is NOT_FOUND before any state gating
    commandData.analysisStreamID = 77;
    auto response                = mServer.GetLogic().HandleDeactivateAnalysisStream(commandHandler, kCommandPath, commandData);
    if (response.has_value())
    {
        ASSERT_TRUE(response->GetStatusCode() == Protocols::InteractionModel::ClusterStatusCode(Status::NotFound));
    }
    else
    {
        FAIL();
    }

    // TODO: deactivation is not implemented yet; no stream can be in an active state,
    // so INVALID_IN_STATE is sent as response for a known stream.
    Testing::MockCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    commandData.analysisStreamID = 0;
    response                     = mServer.GetLogic().HandleDeactivateAnalysisStream(commandHandler, kCommandPath, commandData);
    if (response.has_value())
    {
        ASSERT_TRUE(response->GetStatusCode() == Protocols::InteractionModel::ClusterStatusCode(Status::InvalidInState));
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
    commandData.analysisStreamID = 0; // cluster-assigned id; the camera's id 42 stays internal
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

TEST_F(TestRemoteAvAnalysisCluster, RemoveAnalysisStreamRetiresEntryButPropagatesCameraNotFound)
{
    Testing::MockCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    Testing::MockCommandHandler removeHandler;
    removeHandler.SetFabricIndex(1);

    auto response = mServer.GetLogic().HandleRemoveAnalysisStream(removeHandler, path, commandData);
    ASSERT_FALSE(response.has_value());

    // The camera no longer has the stream. Spec 11.9.8.7.2 wants its status verbatim, but keeping
    // the entry would occupy capacity that no retry could ever free.
    mFakeCameraClient.mLastCallback->OnVideoStreamDeallocated(Status::NotFound, 42);

    ASSERT_EQ(removeHandler.GetLastStatus().status.GetStatus(), Status::NotFound);

    uint8_t currentCount = 0xFF;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 0);
}

TEST_F(TestRemoteAvAnalysisCluster, RemoveAnalysisStreamPropagatesCameraFailure)
{
    Testing::MockCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
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
    ASSERT_EQ(iter.GetValue().analysisStreamID, 0);
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kPendingInitiation);

    // The restored stream is still bound to its camera: removing it deallocates on that camera
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    Testing::MockCommandHandler removeHandler;
    removeHandler.SetFabricIndex(1);
    auto response = mServer.GetLogic().HandleRemoveAnalysisStream(removeHandler, path, commandData);
    ASSERT_FALSE(response.has_value());
    ASSERT_EQ(mFakeCameraClient.mLastCamera, ScopedNodeId(kCameraNodeId, 1));

    mFakeCameraClient.mLastCallback->OnVideoStreamDeallocated(Status::Success, 42);
    ASSERT_TRUE(removeHandler.HasStatus());
}

TEST_F(TestRemoteAvAnalysisCluster, WritingTrackingEnabledWithTheSameValueIsNotReported)
{
    bool trackingEnabled = false;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::TrackingEnabled::Id, trackingEnabled), CHIP_NO_ERROR);

    ASSERT_EQ(mServer.GetLogic().SetTrackingEnabled(!trackingEnabled), CHIP_NO_ERROR);
    const size_t dirtyAfterChange = mClusterTester.GetDirtyList().size();

    // Re-writing the value it already holds must not report or persist again
    ASSERT_EQ(mServer.GetLogic().SetTrackingEnabled(!trackingEnabled), CHIP_NO_ERROR);
    ASSERT_EQ(mClusterTester.GetDirtyList().size(), dirtyAfterChange);
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

// Simulates the real CommandHandlerImpl handle lifecycle: tracks Handles and can invalidate them,
// as happens when the invoking client's exchange closes (e.g. client-side timeout).
class InvalidatableCommandHandler : public Testing::MockCommandHandler
{
public:
    void IncrementHoldOff(CommandHandler::Handle * apHandle) override { mHandles.push_back(apHandle); }
    void DecrementHoldOff(CommandHandler::Handle * apHandle) override
    {
        mHandles.erase(std::remove(mHandles.begin(), mHandles.end(), apHandle), mHandles.end());
    }

    void InvalidateHandles()
    {
        for (auto * handle : mHandles)
        {
            handle->Invalidate();
        }
        mHandles.clear();
    }

private:
    std::vector<CommandHandler::Handle *> mHandles;
};

TEST_F(TestRemoteAvAnalysisCluster, BusyWhileInteractionInFlightEvenIfCommandExchangeDied)
{
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
    Commands::EstablishAnalysisStream::DecodableType commandData;
    commandData.nodeID = 0x1234;

    // First command parks and starts the camera interaction
    InvalidatableCommandHandler firstHandler;
    firstHandler.SetFabricIndex(1);
    auto firstResponse = mServer.GetLogic().HandleEstablishAnalysisStream(firstHandler, path, commandData);
    ASSERT_FALSE(firstResponse.has_value());
    auto * firstCallback = mFakeCameraClient.mLastCallback;

    // The client's exchange dies (e.g. chip-tool timeout) while the camera interaction continues
    firstHandler.InvalidateHandles();

    // A second command must still be answered Busy:
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

    // The first interaction's late completion must not answer the second command
    firstCallback->OnVideoStreamAllocated(Status::Success, 7);
    ASSERT_FALSE(secondHandler.HasResponse());
    ASSERT_FALSE(secondHandler.HasStatus());

    // With the interaction complete, a new command is accepted again
    Testing::MockCommandHandler thirdHandler;
    thirdHandler.SetFabricIndex(1);
    auto thirdResponse = mServer.GetLogic().HandleEstablishAnalysisStream(thirdHandler, path, commandData);
    ASSERT_FALSE(thirdResponse.has_value());
    mFakeCameraClient.mLastCallback->OnVideoStreamAllocated(Status::Success, 8);
    ASSERT_TRUE(thirdHandler.HasResponse());
}

TEST_F(TestRemoteAvAnalysisCluster, DeadExchangeAllocationStillTracksCameraStream)
{
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
    Commands::EstablishAnalysisStream::DecodableType commandData;
    commandData.nodeID = 0x1234;

    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    auto response = mServer.GetLogic().HandleEstablishAnalysisStream(commandHandler, path, commandData);
    ASSERT_FALSE(response.has_value());

    // The client exchange dies, then the camera completes the allocation successfully. The camera
    // stream is real regardless of the dead exchange and must be tracked, or it leaks on the camera
    // with no way to ever remove it.
    commandHandler.InvalidateHandles();
    mFakeCameraClient.mLastCallback->OnVideoStreamAllocated(Status::Success, 7);

    uint8_t currentCount = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 1);

    // The tracked stream can be removed normally afterwards (cluster-assigned id 0; camera id 7 is internal)
    ConcreteCommandPath removePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType removeData;
    removeData.analysisStreamID = 0;
    Testing::MockCommandHandler removeHandler;
    removeHandler.SetFabricIndex(1);
    auto removeResponse = mServer.GetLogic().HandleRemoveAnalysisStream(removeHandler, removePath, removeData);
    ASSERT_FALSE(removeResponse.has_value());
    mFakeCameraClient.mLastCallback->OnVideoStreamDeallocated(Status::Success, 7);

    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 0);
}

TEST_F(TestRemoteAvAnalysisCluster, DeadExchangeDeallocationStillRemovesEntry)
{
    // Establish a stream normally
    Testing::MockCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 9);

    // Remove it, but the client exchange dies before the camera answers
    ConcreteCommandPath removePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType removeData;
    removeData.analysisStreamID = 0; // cluster-assigned id; the camera's id 9 stays internal
    InvalidatableCommandHandler removeHandler;
    removeHandler.SetFabricIndex(1);
    auto response = mServer.GetLogic().HandleRemoveAnalysisStream(removeHandler, removePath, removeData);
    ASSERT_FALSE(response.has_value());
    removeHandler.InvalidateHandles();

    // The camera has deallocated its stream; the entry must not survive as a permanently stuck slot
    mFakeCameraClient.mLastCallback->OnVideoStreamDeallocated(Status::Success, 9);

    uint8_t currentCount = 0xFF;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 0);
}

TEST_F(TestRemoteAvAnalysisCluster, EstablishSameCameraStreamIsIdempotent)
{
    // First stream established with camera-assigned id 42, cluster-assigned AnalysisStreamID 0
    Testing::MockCommandHandler firstHandler;
    firstHandler.SetFabricIndex(1);
    EstablishStream(firstHandler, 0x1234, Status::Success, 42);

    // The camera hands out the same id again, the retry is answered with the existing analysis stream
    // instead of creating a second entry over the same camera stream
    Testing::MockCommandHandler secondHandler;
    secondHandler.SetFabricIndex(1);
    EstablishStream(secondHandler, 0x1234, Status::Success, 42);

    ASSERT_TRUE(secondHandler.HasResponse());
    Commands::EstablishAnalysisStreamResponse::DecodableType response;
    ASSERT_EQ(secondHandler.DecodeResponse(response), CHIP_NO_ERROR);
    ASSERT_EQ(response.analysisStreamID, 0);
    ASSERT_EQ(mFakeCameraClient.mDeallocationRequests, 0);

    uint8_t currentCount = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 1);

    // A different camera answering with the same VideoStreamID is a distinct stream: new entry
    Testing::MockCommandHandler thirdHandler;
    thirdHandler.SetFabricIndex(1);
    EstablishStream(thirdHandler, 0x5678, Status::Success, 42);
    ASSERT_TRUE(thirdHandler.HasResponse());
    ASSERT_EQ(thirdHandler.DecodeResponse(response), CHIP_NO_ERROR);
    ASSERT_EQ(response.analysisStreamID, 1);
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 2);
}

TEST_F(TestRemoteAvAnalysisCluster, ShutdownWithoutInteractionLeavesTheCameraClientAlone)
{
    // The camera client may be shared with other AV Analysis endpoints; shutting this cluster down
    // with nothing of ours in flight must not cancel another endpoint's request.
    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);
    ASSERT_EQ(mFakeCameraClient.mCancelCount, 0);

    // Restart so the fixture TearDown shuts down a running server
    EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
}

TEST_F(TestRemoteAvAnalysisCluster, ShutdownCancelsPendingCameraInteraction)
{
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
    Commands::EstablishAnalysisStream::DecodableType commandData;
    commandData.nodeID = 0x1234;
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);

    auto response = mServer.GetLogic().HandleEstablishAnalysisStream(commandHandler, path, commandData);
    ASSERT_FALSE(response.has_value()); // Pending on the camera
    auto * pendingCallback = mFakeCameraClient.mLastCallback;
    ASSERT_NE(pendingCallback, nullptr);

    // Shutting down with the interaction in flight must cancel it (the callback refers to the logic)
    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);
    ASSERT_GE(mFakeCameraClient.mCancelCount, 1);

    // A stray late completion (contract violation by a client) must be a harmless no-op
    pendingCallback->OnVideoStreamAllocated(Status::Success, 42);
    ASSERT_FALSE(commandHandler.HasResponse());

    // Restart so the fixture TearDown shuts down a running server
    EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
}

TEST_F(TestRemoteAvAnalysisCluster, ShutdownAnswersTheParkedCommand)
{
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
    Commands::EstablishAnalysisStream::DecodableType commandData;
    commandData.nodeID = 0x1234;
    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);

    auto response = mServer.GetLogic().HandleEstablishAnalysisStream(commandHandler, path, commandData);
    ASSERT_FALSE(response.has_value()); // Pending on the camera

    // On a cluster-only shutdown the invoking client is still there; it must be answered rather
    // than left waiting for the interaction to time out
    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);

    ASSERT_TRUE(commandHandler.HasStatus());
    ASSERT_EQ(commandHandler.GetLastStatus().status.GetStatus(), Status::Failure);

    // Restart so the fixture TearDown shuts down a running server
    EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
}

TEST_F(TestRemoteAvAnalysisCluster, AnalysisStreamTableEncodeDecodeTest)
{
    AnalysisStreamTable table;
    ASSERT_EQ(table.Init(3), CHIP_NO_ERROR);

    // AnalysisStreamIDs are generated monotonically; the camera-assigned VideoStreamID is stored per entry
    const ScopedNodeId kCamera(0xABCD, 1);
    AnalysisStreamEntry * first = table.Add(10, kCamera);
    ASSERT_NE(first, nullptr);
    ASSERT_EQ(first->analysisStreamID, 0);
    ASSERT_EQ(table.Add(20, kCamera)->analysisStreamID, 1);
    ASSERT_EQ(table.Add(30, kCamera)->analysisStreamID, 2);
    ASSERT_EQ(table.Add(40, kCamera), nullptr); // table full
    ASSERT_EQ(table.Count(), 3);

    // One camera stream maps to at most one entry
    ASSERT_EQ(table.FindByCameraStream(kCamera, 20)->analysisStreamID, 1);
    ASSERT_EQ(table.FindByCameraStream(ScopedNodeId(0x9999, 1), 20), nullptr);

    // Give one entry an active session so decode's state reset is observable
    AnalysisStreamEntry * active = table.Find(1);
    ASSERT_NE(active, nullptr);
    active->state            = AnalysisStreamStateEnum::kWebRTCActive;
    active->webRTCEndpointID = DataModel::MakeNullable<EndpointId>(2);

    uint8_t buffer[AnalysisStreamTable::kEntrySerializedSize * 3 + AnalysisStreamTable::kArraySerializedOverhead];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));
    ASSERT_EQ(table.Encode(writer), CHIP_NO_ERROR);

    // Restore into a larger table so the id-counter check below has room for one more entry
    AnalysisStreamTable restored;
    ASSERT_EQ(restored.Init(4), CHIP_NO_ERROR);
    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ASSERT_EQ(restored.Decode(reader), CHIP_NO_ERROR);

    // All entries survive; sessions do not survive a reboot, so state collapses to PendingInitiation
    ASSERT_EQ(restored.Count(), 3);
    struct
    {
        uint16_t analysisId;
        uint16_t videoId;
    } expected[] = { { 0, 10 }, { 1, 20 }, { 2, 30 } };
    for (const auto & e : expected)
    {
        AnalysisStreamEntry * entry = restored.Find(e.analysisId);
        ASSERT_NE(entry, nullptr);
        ASSERT_EQ(entry->videoStreamID, e.videoId); // The camera stream mapping must survive reboot
        ASSERT_EQ(entry->state, AnalysisStreamStateEnum::kPendingInitiation);
        ASSERT_TRUE(entry->webRTCEndpointID.IsNull());
        ASSERT_TRUE(entry->pushAVEndpointID.IsNull());
        ASSERT_EQ(entry->cameraNode, kCamera); // The camera association must survive reboot
    }

    // The id counter survives too: the next id does not collide with restored entries
    AnalysisStreamEntry * added = restored.Add(50, kCamera);
    ASSERT_NE(added, nullptr);
    ASSERT_EQ(added->analysisStreamID, 3);
}

TEST_F(TestRemoteAvAnalysisCluster, AnalysisStreamTableDecodeRejectsDuplicateIds)
{
    AnalysisStreamTable table;
    ASSERT_EQ(table.Init(2), CHIP_NO_ERROR);
    const ScopedNodeId kCamera(0xABCD, 1);
    ASSERT_NE(table.Add(10, kCamera), nullptr);
    ASSERT_NE(table.Add(20, kCamera), nullptr);

    // Forge a blob this code would never write: two entries sharing one AnalysisStreamID
    table[1].analysisStreamID = table[0].analysisStreamID;

    uint8_t buffer[AnalysisStreamTable::kEntrySerializedSize * 2 + AnalysisStreamTable::kArraySerializedOverhead];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));
    ASSERT_EQ(table.Encode(writer), CHIP_NO_ERROR);

    AnalysisStreamTable restored;
    ASSERT_EQ(restored.Init(2), CHIP_NO_ERROR);
    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ASSERT_EQ(restored.Decode(reader), CHIP_ERROR_INVALID_ARGUMENT);

    // The rejected blob degrades to an empty table, same as any other corruption
    ASSERT_EQ(restored.Count(), 0);
}

TEST_F(TestRemoteAvAnalysisCluster, AnalysisStreamTableDecodeRejectsOutOfRangeIdCounter)
{
    // Hand-build a blob in Encode()'s layout whose next-id counter is 65535: the generator never
    // leaves the counter above 65534 (the AnalysisStreamID ceiling of spec 11.9.5.3), so accepting
    // such a blob would mint one out-of-range id.
    uint8_t buffer[32];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));
    TLV::TLVType outerType;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType), CHIP_NO_ERROR);
    ASSERT_EQ(writer.Put(TLV::ContextTag(0), static_cast<uint16_t>(65535)), CHIP_NO_ERROR);
    TLV::TLVType arrayType;
    ASSERT_EQ(writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Array, arrayType), CHIP_NO_ERROR);
    ASSERT_EQ(writer.EndContainer(arrayType), CHIP_NO_ERROR);
    ASSERT_EQ(writer.EndContainer(outerType), CHIP_NO_ERROR);

    AnalysisStreamTable restored;
    ASSERT_EQ(restored.Init(2), CHIP_NO_ERROR);
    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ASSERT_EQ(restored.Decode(reader), CHIP_ERROR_INVALID_ARGUMENT);

    // The rejected blob leaves a usable table minting ids from the start again
    AnalysisStreamEntry * added = restored.Add(10, ScopedNodeId(0xABCD, 1));
    ASSERT_NE(added, nullptr);
    ASSERT_EQ(added->analysisStreamID, 0);
}

TEST_F(TestRemoteAvAnalysisCluster, AnalysisStreamTableDecodeFailureLeavesTableEmpty)
{
    AnalysisStreamTable table;
    ASSERT_EQ(table.Init(3), CHIP_NO_ERROR);
    const ScopedNodeId kCamera(0xABCD, 1);
    ASSERT_NE(table.Add(10, kCamera), nullptr);
    ASSERT_NE(table.Add(20, kCamera), nullptr);
    ASSERT_NE(table.Add(30, kCamera), nullptr);

    uint8_t buffer[AnalysisStreamTable::kEntrySerializedSize * 3 + AnalysisStreamTable::kArraySerializedOverhead];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));
    ASSERT_EQ(table.Encode(writer), CHIP_NO_ERROR);

    // Corrupt the blob: cut it off inside the second entry, after the first decoded cleanly
    const uint32_t truncatedLength = writer.GetLengthWritten() - static_cast<uint32_t>(AnalysisStreamTable::kEntrySerializedSize);

    AnalysisStreamTable restored;
    ASSERT_EQ(restored.Init(3), CHIP_NO_ERROR);
    TLV::TLVReader reader;
    reader.Init(buffer, truncatedLength);
    ASSERT_NE(restored.Decode(reader), CHIP_NO_ERROR);

    // A corrupt blob degrades to an empty table, never a half-loaded one
    ASSERT_EQ(restored.Count(), 0);

    // The reset table is fully usable and mints ids from the start again
    AnalysisStreamEntry * added = restored.Add(40, kCamera);
    ASSERT_NE(added, nullptr);
    ASSERT_EQ(added->analysisStreamID, 0);
    ASSERT_EQ(restored.Count(), 1);
}
} // namespace
