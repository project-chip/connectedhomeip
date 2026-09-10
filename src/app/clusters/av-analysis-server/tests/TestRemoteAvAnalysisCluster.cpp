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
#include <algorithm>
#include <optional>
#include <pw_unit_test/framework.h>

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
#include <lib/support/DefaultStorageKeyAllocator.h>
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
// A command handler's response, for assertions. clang-tidy does not treat ASSERT_TRUE(has_value())
// as a guard (bugprone-unchecked-optional-access), so the optional is read through value_or: an
// absent response reads as Failure and fails the assertion that follows.
chip::app::DataModel::ActionReturnStatus StatusOf(const std::optional<chip::app::DataModel::ActionReturnStatus> & aResponse)
{
    return aResponse.value_or(chip::app::DataModel::ActionReturnStatus(chip::Protocols::InteractionModel::Status::Failure));
}

const std::vector<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> testAmbientContexts = {
    { std::nullopt, static_cast<uint8_t>(0x49), static_cast<uint8_t>(0x0B),
      MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Object.Package"_span)) },
    { std::nullopt, static_cast<uint8_t>(0x4B), static_cast<uint8_t>(0x08),
      MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Activity.Delivery"_span)) },
    { std::nullopt, static_cast<uint8_t>(0x4B), static_cast<uint8_t>(0x09),
      MakeOptional(chip::app::DataModel::Nullable<chip::CharSpan>("Activity.Retrieval"_span)) }
};

// A mock handler that tracks the Handles parked on it: a dead client exchange is simulated by
// invalidating them, and none is left dangling once the handler goes out of scope
class InvalidatableCommandHandler : public Testing::MockCommandHandler
{
public:
    ~InvalidatableCommandHandler() override { InvalidateHandles(); }

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

        // What the camera-bound requests answer; a failure starts nothing and reports nothing
        CHIP_ERROR mAllocationResult   = CHIP_NO_ERROR;
        CHIP_ERROR mDeallocationResult = CHIP_NO_ERROR;

        CHIP_ERROR RequestVideoStreamAllocation(const ScopedNodeId & aCameraNode, Callback & aCallback) override
        {
            ReturnErrorOnFailure(mAllocationResult);
            mAllocationRequests++;
            mLastCamera   = aCameraNode;
            mLastCallback = &aCallback;
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR RequestVideoStreamDeallocation(const ScopedNodeId & aCameraNode, uint16_t aVideoStreamId,
                                                  Callback & aCallback) override
        {
            ReturnErrorOnFailure(mDeallocationResult);
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

    // Records WebRTC session interactions started by the command handlers; tests complete them by
    // invoking the recorded Callback, simulating the camera's side of the signaling.
    class FakeWebRTCClient : public AvAnalysisWebRTCClient
    {
    public:
        int mSessionRequests = 0;
        int mEndRequests     = 0;
        ScopedNodeId mLastCamera;
        EndpointId mLastEndpoint  = kInvalidEndpointId;
        uint16_t mLastVideoStream = 0;
        uint16_t mLastSessionId   = 0;
        Callback * mLastCallback  = nullptr;

        CHIP_ERROR RequestSession(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint, uint16_t aVideoStreamId,
                                  Callback & aCallback) override
        {
            mSessionRequests++;
            mLastCamera      = aCameraNode;
            mLastEndpoint    = aWebRTCEndpoint;
            mLastVideoStream = aVideoStreamId;
            mLastCallback    = &aCallback;
            return mRequestSessionResult;
        }

        // What RequestSession() returns; a failure means the request never started
        CHIP_ERROR mRequestSessionResult = CHIP_NO_ERROR;

        CHIP_ERROR EndSession(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint, uint16_t aWebRTCSessionId,
                              Callback & aCallback) override
        {
            mEndRequests++;
            mLastCamera    = aCameraNode;
            mLastEndpoint  = aWebRTCEndpoint;
            mLastSessionId = aWebRTCSessionId;
            mLastCallback  = &aCallback;
            ReturnErrorOnFailure(mEndSessionResult);

            // A real client completes synchronously when the CASE session is cached and the send
            // fails at once: the outcome arrives before EndSession() returns success
            if (mSynchronousEndOutcome.has_value())
            {
                aCallback.OnSessionEnded(*mSynchronousEndOutcome, aWebRTCSessionId);
            }
            return CHIP_NO_ERROR;
        }

        // What EndSession() returns; a failure means the request never started
        CHIP_ERROR mEndSessionResult = CHIP_NO_ERROR;
        // When set, EndSession() delivers this outcome before returning
        std::optional<Status> mSynchronousEndOutcome;

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
        mServer.SetWebRTCClient(&mFakeWebRTCClient);
        EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
        EXPECT_EQ(mServer.Init(), CHIP_NO_ERROR);
    }

    void TearDown() override { mServer.Shutdown(ClusterShutdownType::kClusterShutdown); }

    // One entry of a ContextTriggers command list, in an encodable-friendly shape
    struct TestTrigger
    {
        Descriptor::Structs::SemanticTagStruct::Type context;
        DataModel::Nullable<std::vector<uint16_t>> zones;
    };

    // Encodes a ContextTriggers list into aTlvBuffer and returns it as the nullable decodable
    // list the command payload carries. aTlvBuffer must outlive the returned list.
    DataModel::Nullable<DataModel::DecodableList<Structs::ContextTriggerStruct::DecodableType>>
    EncodeContextTriggers(const std::vector<TestTrigger> & aTriggers, uint8_t * aTlvBuffer, size_t aTlvBufferSize)
    {
        TLV::TLVWriter writer;
        writer.Init(aTlvBuffer, static_cast<uint32_t>(aTlvBufferSize));
        TLV::TLVType arrayType;
        EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType), CHIP_NO_ERROR);
        for (const TestTrigger & spec : aTriggers)
        {
            Structs::ContextTriggerStruct::Type trigger;
            trigger.context = spec.context;
            if (spec.zones.IsNull())
            {
                trigger.zoneIDs = MakeOptional(DataModel::NullNullable);
            }
            else
            {
                trigger.zoneIDs = MakeOptional(
                    DataModel::MakeNullable(DataModel::List<const uint16_t>(spec.zones.Value().data(), spec.zones.Value().size())));
            }
            EXPECT_EQ(DataModel::Encode(writer, TLV::AnonymousTag(), trigger), CHIP_NO_ERROR);
        }
        EXPECT_EQ(writer.EndContainer(arrayType), CHIP_NO_ERROR);

        TLV::TLVReader reader;
        reader.Init(aTlvBuffer, writer.GetLengthWritten());
        EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
        DataModel::DecodableList<Structs::ContextTriggerStruct::DecodableType> decodedList;
        EXPECT_EQ(decodedList.Decode(reader), CHIP_NO_ERROR);
        return DataModel::MakeNullable(decodedList);
    }

    DataModel::Nullable<DataModel::DecodableList<Structs::ContextTriggerStruct::DecodableType>>
    EncodeContextTriggers(const Descriptor::Structs::SemanticTagStruct::Type & aContext,
                          const DataModel::Nullable<std::vector<uint16_t>> & aZones, uint8_t * aTlvBuffer, size_t aTlvBufferSize)
    {
        return EncodeContextTriggers(std::vector<TestTrigger>{ { aContext, aZones } }, aTlvBuffer, aTlvBufferSize);
    }

    // Sends EstablishAnalysisStream for the given camera node and completes the camera allocation
    // with the given status/stream id; returns the mock handler carrying response or status.
    void EstablishStream(InvalidatableCommandHandler & commandHandler, NodeId aCameraNodeId, Status aCameraStatus,
                         uint16_t aVideoStreamId)
    {
        ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
        Commands::EstablishAnalysisStream::DecodableType commandData;
        commandData.nodeID = aCameraNodeId;

        auto response = mServer.GetLogic().HandleEstablishAnalysisStream(commandHandler, path, commandData);
        EXPECT_FALSE(response.has_value()); // Response is pending on the camera interaction
        EXPECT_NE(mFakeCameraClient.mLastCallback, nullptr);
        if (mFakeCameraClient.mLastCallback != nullptr)
        {
            mFakeCameraClient.mLastCallback->OnVideoStreamAllocated(aCameraStatus, aVideoStreamId);
        }
    }

    // Establishes-then-activates stream aAnalysisStreamId through the fakes: the offer exchange
    // succeeds with aSessionId and the session goes active.
    void ActivateStream(uint16_t aAnalysisStreamId, EndpointId aWebRTCEndpoint, uint16_t aSessionId)
    {
        ActivateStream(aAnalysisStreamId, aWebRTCEndpoint, aSessionId, ScopedNodeId(0x1234, 1));
    }

    void ActivateStream(uint16_t aAnalysisStreamId, EndpointId aWebRTCEndpoint, uint16_t aSessionId,
                        const ScopedNodeId & aCameraNode)
    {
        InvalidatableCommandHandler activateHandler;
        activateHandler.SetFabricIndex(1);
        ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
        Commands::ActivateAnalysisStream::DecodableType commandData;
        commandData.analysisStreamID = aAnalysisStreamId;
        commandData.webRTCEndpointID = MakeOptional(aWebRTCEndpoint);

        auto response = mServer.GetLogic().HandleActivateAnalysisStream(activateHandler, path, commandData);
        EXPECT_FALSE(response.has_value());
        EXPECT_NE(mFakeWebRTCClient.mLastCallback, nullptr);
        if (mFakeWebRTCClient.mLastCallback != nullptr)
        {
            mFakeWebRTCClient.mLastCallback->OnSessionInitiated(Status::Success, aSessionId, /* aOfferSent = */ true);
            EXPECT_EQ(LastStatus(activateHandler), Status::Success);
            mFakeWebRTCClient.mLastCallback->OnSessionActive(aCameraNode, aSessionId);
        }

        // What every caller of this helper goes on to assume
        EXPECT_EQ(StreamState(aAnalysisStreamId), AnalysisStreamStateEnum::kWebRTCActive);
    }

    template <typename RequestType>
    Status InvokedStatus(CommandId aCommandId, const RequestType & aRequest)
    {
        auto result = mClusterTester.Invoke(aCommandId, aRequest);
        auto code   = result.GetStatusCode();
        EXPECT_TRUE(code.has_value()) << "command 0x" << std::hex << aCommandId << " answered no status";
        return code.has_value() ? code->GetStatus() : Status::InvalidAction;
    }

    Status ImmediateActivateStatus(uint16_t aAnalysisStreamId, EndpointId aWebRTCEndpoint)
    {
        InvalidatableCommandHandler handler;
        handler.SetFabricIndex(1);
        ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
        Commands::ActivateAnalysisStream::DecodableType commandData;
        commandData.analysisStreamID = aAnalysisStreamId;
        commandData.webRTCEndpointID = MakeOptional(aWebRTCEndpoint);
        auto response                = mServer.GetLogic().HandleActivateAnalysisStream(handler, path, commandData);
        EXPECT_TRUE(response.has_value()) << "ActivateAnalysisStream unexpectedly parked";
        return StatusOf(response).GetStatusCode().GetStatus();
    }

    Status ImmediateDeactivateStatus(uint16_t aAnalysisStreamId)
    {
        InvalidatableCommandHandler handler;
        handler.SetFabricIndex(1);
        ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
        Commands::DeactivateAnalysisStream::DecodableType commandData;
        commandData.analysisStreamID = aAnalysisStreamId;
        auto response                = mServer.GetLogic().HandleDeactivateAnalysisStream(handler, path, commandData);
        EXPECT_TRUE(response.has_value()) << "DeactivateAnalysisStream unexpectedly parked";
        return StatusOf(response).GetStatusCode().GetStatus();
    }

    Status ImmediateRemoveStatus(uint16_t aAnalysisStreamId)
    {
        InvalidatableCommandHandler handler;
        handler.SetFabricIndex(1);
        ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
        Commands::RemoveAnalysisStream::DecodableType commandData;
        commandData.analysisStreamID = aAnalysisStreamId;
        auto response                = mServer.GetLogic().HandleRemoveAnalysisStream(handler, path, commandData);
        EXPECT_TRUE(response.has_value()) << "RemoveAnalysisStream unexpectedly parked";
        return StatusOf(response).GetStatusCode().GetStatus();
    }

    // The last status recorded on aHandler, or InvalidAction when it was never answered: no handler
    // is ever answered InvalidAction, so an unanswered command fails the comparison rather than
    // reading an empty vector
    static Status LastStatus(const Testing::MockCommandHandler & aHandler)
    {
        return aHandler.HasStatus() ? aHandler.GetLastStatus().status.GetStatus() : Status::InvalidAction;
    }

    // The state of the first stream in the AnalysisStreams attribute
    // The state of one stream by id, kUnknownEnumValue if there is no such entry: degrades rather
    // than returning, so it is usable from the fixture helpers
    AnalysisStreamStateEnum StreamState(uint16_t aAnalysisStreamId)
    {
        Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
        if (mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams) != CHIP_NO_ERROR)
        {
            return AnalysisStreamStateEnum::kUnknownEnumValue;
        }
        auto iter = streams.begin();
        while (iter.Next())
        {
            if (iter.GetValue().analysisStreamID == aAnalysisStreamId)
            {
                return iter.GetValue().analysisStreamState;
            }
        }
        return AnalysisStreamStateEnum::kUnknownEnumValue;
    }

    AnalysisStreamStateEnum FirstStreamState()
    {
        Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
        if (mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams) != CHIP_NO_ERROR)
        {
            return AnalysisStreamStateEnum::kUnknownEnumValue;
        }
        auto iter = streams.begin();
        if (!iter.Next())
        {
            return AnalysisStreamStateEnum::kUnknownEnumValue;
        }
        return iter.GetValue().analysisStreamState;
    }

    MockAvAnalysisDelegate mMockDelegate;
    FakeCameraClient mFakeCameraClient;
    FakeWebRTCClient mFakeWebRTCClient;
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
    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(mServer, { Commands::EstablishAnalysisStreamResponse::Id }));
}

TEST_F(TestRemoteAvAnalysisCluster, EnableContextTriggersRequiresAnEstablishedStream)
{
    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;
    commandData.contextTriggers.SetNull();

    // RemoteContextDetection and no established analysis stream, the command returns INVALID_IN_STATE
    auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, path, commandData);
    ASSERT_TRUE(response.has_value());
    ASSERT_EQ(StatusOf(response).GetStatusCode().GetStatus(), Status::InvalidInState);

    // No context triggers were enabled by the rejected command
    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType active;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, active), CHIP_NO_ERROR);
    size_t count = 0;
    ASSERT_EQ(active.ComputeSize(&count), CHIP_NO_ERROR);
    ASSERT_EQ(count, 0u);

    // Once a stream is established the same command enables all the context triggers
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 7);

    response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, path, commandData);
    ASSERT_TRUE(response.has_value());
    ASSERT_TRUE(StatusOf(response).IsSuccess());

    // Null ContextTriggers means the whole supported set is enabled
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, active), CHIP_NO_ERROR);
    ASSERT_EQ(active.ComputeSize(&count), CHIP_NO_ERROR);
    ASSERT_EQ(count, testAmbientContexts.size());
}

TEST_F(TestRemoteAvAnalysisCluster, EnableContextTriggersEnablesASpecificContext)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 7);

    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;
    uint8_t tlvBuffer[128];
    const std::vector<uint16_t> zoneIDs = { 1, 2 };
    commandData.contextTriggers =
        EncodeContextTriggers(testAmbientContexts.front(), DataModel::MakeNullable(zoneIDs), tlvBuffer, sizeof(tlvBuffer));

    auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, path, commandData);
    ASSERT_TRUE(response.has_value());
    ASSERT_TRUE(StatusOf(response).IsSuccess());

    // Exactly the requested context is enabled, carrying its zone list
    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType active;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, active), CHIP_NO_ERROR);
    auto iter = active.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().context.namespaceID, testAmbientContexts.front().namespaceID);
    ASSERT_EQ(iter.GetValue().context.tag, testAmbientContexts.front().tag);
    ASSERT_TRUE(iter.GetValue().zoneIDs.HasValue());
    ASSERT_FALSE(iter.GetValue().zoneIDs.Value().IsNull());
    std::vector<uint16_t> readZones;
    auto zoneIter = iter.GetValue().zoneIDs.Value().Value().begin();
    while (zoneIter.Next())
    {
        readZones.push_back(zoneIter.GetValue());
    }
    ASSERT_EQ(readZones, zoneIDs);
    ASSERT_FALSE(iter.Next());
}

TEST_F(TestRemoteAvAnalysisCluster, EnableContextTriggersRejectsAnUnsupportedContext)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 7);

    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;

    // A context outside SupportedAmbientContexts (Sound.Snoring is not in the test set)
    Descriptor::Structs::SemanticTagStruct::Type unsupportedContext = { std::nullopt, static_cast<uint8_t>(0x4A),
                                                                        static_cast<uint8_t>(0x02), NullOptional };
    uint8_t tlvBuffer[128];
    commandData.contextTriggers = EncodeContextTriggers(unsupportedContext, DataModel::NullNullable, tlvBuffer, sizeof(tlvBuffer));

    auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, path, commandData);
    ASSERT_TRUE(response.has_value());
    ASSERT_EQ(StatusOf(response).GetStatusCode().GetStatus(), Status::ConstraintError);
}

TEST_F(TestRemoteAvAnalysisCluster, EnableContextTriggersAppliesNothingWhenAnyTriggerIsInvalid)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 7);

    // A valid trigger followed by an unsupported one: every validation failure must end
    // processing with no other side-effects, so the valid trigger must not be armed either.
    Descriptor::Structs::SemanticTagStruct::Type unsupportedContext = { std::nullopt, static_cast<uint8_t>(0x4A),
                                                                        static_cast<uint8_t>(0x02), NullOptional };
    std::vector<TestTrigger> triggers                               = {
        { testAmbientContexts.front(), DataModel::MakeNullable(std::vector<uint16_t>{ 1, 2 }) },
        { unsupportedContext, DataModel::NullNullable },
    };
    uint8_t tlvBuffer[256];
    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;
    commandData.contextTriggers = EncodeContextTriggers(triggers, tlvBuffer, sizeof(tlvBuffer));

    auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, path, commandData);
    ASSERT_TRUE(response.has_value());
    ASSERT_EQ(StatusOf(response).GetStatusCode().GetStatus(), Status::ConstraintError);

    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType active;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, active), CHIP_NO_ERROR);
    size_t count = 1;
    ASSERT_EQ(active.ComputeSize(&count), CHIP_NO_ERROR);
    ASSERT_EQ(count, 0u);
}

TEST_F(TestRemoteAvAnalysisCluster, EnableContextTriggersIgnoresDuplicateEntries)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 7);

    // The same context twice with different zones: duplicates are ignored, the first wins
    std::vector<TestTrigger> triggers = {
        { testAmbientContexts.front(), DataModel::MakeNullable(std::vector<uint16_t>{ 1, 2 }) },
        { testAmbientContexts.front(), DataModel::MakeNullable(std::vector<uint16_t>{ 3 }) },
    };
    uint8_t tlvBuffer[256];
    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;
    commandData.contextTriggers = EncodeContextTriggers(triggers, tlvBuffer, sizeof(tlvBuffer));

    auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, path, commandData);
    ASSERT_TRUE(response.has_value());
    ASSERT_TRUE(StatusOf(response).IsSuccess());

    // Exactly one entry, carrying the first occurrence's zone list
    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType active;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, active), CHIP_NO_ERROR);
    auto iter = active.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_TRUE(iter.GetValue().zoneIDs.HasValue());
    ASSERT_FALSE(iter.GetValue().zoneIDs.Value().IsNull());
    std::vector<uint16_t> readZones;
    auto zoneIter = iter.GetValue().zoneIDs.Value().Value().begin();
    while (zoneIter.Next())
    {
        readZones.push_back(zoneIter.GetValue());
    }
    ASSERT_EQ(readZones, (std::vector<uint16_t>{ 1, 2 }));
    ASSERT_FALSE(iter.Next());
}

TEST_F(TestRemoteAvAnalysisCluster, EnableContextTriggersUpdatesTheZonesOfAnExistingContext)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 7);

    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;
    uint8_t tlvBuffer[128];
    InvalidatableCommandHandler firstHandler;
    firstHandler.SetFabricIndex(1);
    commandData.contextTriggers = EncodeContextTriggers(
        testAmbientContexts.front(), DataModel::MakeNullable(std::vector<uint16_t>{ 1, 2 }), tlvBuffer, sizeof(tlvBuffer));
    ASSERT_TRUE(StatusOf(mServer.GetLogic().HandleEnableContextTriggers(firstHandler, path, commandData)).IsSuccess());

    // The same context again with other zones: the entry is updated, not duplicated
    InvalidatableCommandHandler secondHandler;
    secondHandler.SetFabricIndex(1);
    commandData.contextTriggers = EncodeContextTriggers(
        testAmbientContexts.front(), DataModel::MakeNullable(std::vector<uint16_t>{ 3 }), tlvBuffer, sizeof(tlvBuffer));
    ASSERT_TRUE(StatusOf(mServer.GetLogic().HandleEnableContextTriggers(secondHandler, path, commandData)).IsSuccess());

    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType active;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, active), CHIP_NO_ERROR);
    auto iter = active.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_TRUE(iter.GetValue().zoneIDs.HasValue());
    ASSERT_FALSE(iter.GetValue().zoneIDs.Value().IsNull());
    std::vector<uint16_t> readZones;
    auto zoneIter = iter.GetValue().zoneIDs.Value().Value().begin();
    while (zoneIter.Next())
    {
        readZones.push_back(zoneIter.GetValue());
    }
    ASSERT_EQ(readZones, (std::vector<uint16_t>{ 3 }));
    ASSERT_FALSE(iter.Next());
}

TEST_F(TestRemoteAvAnalysisCluster, EnableContextTriggersRejectsZonesTheDelegateDoesNotKnow)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 7);

    // Zone Management has no such zone
    mMockDelegate.mZoneVerificationResult = CHIP_ERROR_NOT_FOUND;

    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;
    uint8_t tlvBuffer[128];
    commandData.contextTriggers = EncodeContextTriggers(
        testAmbientContexts.front(), DataModel::MakeNullable(std::vector<uint16_t>{ 9 }), tlvBuffer, sizeof(tlvBuffer));

    auto response = mServer.GetLogic().HandleEnableContextTriggers(commandHandler, path, commandData);
    ASSERT_EQ(StatusOf(response).GetStatusCode().GetStatus(), Status::NotFound);

    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType active;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, active), CHIP_NO_ERROR);
    size_t count = 1;
    ASSERT_EQ(active.ComputeSize(&count), CHIP_NO_ERROR);
    ASSERT_EQ(count, 0u);
}

TEST_F(TestRemoteAvAnalysisCluster, EnableContextTriggersIsExhaustedOnlyOnceTheListIsValid)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 7);

    // The server cannot take more contexts
    mMockDelegate.mCanAddContextTriggers = false;

    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;
    uint8_t tlvBuffer[256];

    // An invalid list is rejected for its content first: validation precedes the capacity check
    Descriptor::Structs::SemanticTagStruct::Type unsupportedContext = { std::nullopt, static_cast<uint8_t>(0x4A),
                                                                        static_cast<uint8_t>(0x02), NullOptional };
    InvalidatableCommandHandler invalidHandler;
    invalidHandler.SetFabricIndex(1);
    commandData.contextTriggers = EncodeContextTriggers(unsupportedContext, DataModel::NullNullable, tlvBuffer, sizeof(tlvBuffer));
    ASSERT_EQ(
        StatusOf(mServer.GetLogic().HandleEnableContextTriggers(invalidHandler, path, commandData)).GetStatusCode().GetStatus(),
        Status::ConstraintError);

    // A valid list is refused for capacity, and nothing is applied
    InvalidatableCommandHandler validHandler;
    validHandler.SetFabricIndex(1);
    commandData.contextTriggers =
        EncodeContextTriggers(testAmbientContexts.front(), DataModel::NullNullable, tlvBuffer, sizeof(tlvBuffer));
    ASSERT_EQ(StatusOf(mServer.GetLogic().HandleEnableContextTriggers(validHandler, path, commandData)).GetStatusCode().GetStatus(),
              Status::ResourceExhausted);

    // So is the whole supported set
    InvalidatableCommandHandler nullHandler;
    nullHandler.SetFabricIndex(1);
    commandData.contextTriggers.SetNull();
    ASSERT_EQ(StatusOf(mServer.GetLogic().HandleEnableContextTriggers(nullHandler, path, commandData)).GetStatusCode().GetStatus(),
              Status::ResourceExhausted);

    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType active;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, active), CHIP_NO_ERROR);
    size_t count = 1;
    ASSERT_EQ(active.ComputeSize(&count), CHIP_NO_ERROR);
    ASSERT_EQ(count, 0u);
}

TEST_F(TestRemoteAvAnalysisCluster, AnalysisSessionRequiresASourceCameraOnARemoteNode)
{
    // Every event of a remote session names the camera the analyzed stream comes from, so a
    // session cannot start without one
    // Through the cluster, so the source parameters it forwards are covered too
    uint16_t sessionId = 0;
    ASSERT_EQ(mServer.AnalysisSessionStart(sessionId, DataModel::NullNullable), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestRemoteAvAnalysisCluster, CreateActiveSessionRequiresASourceCameraOnARemoteNode)
{
    // As AnalysisSessionStart: a remote session's events all name their source, so it cannot be
    // recorded without one
    uint16_t sessionId = 0;
    ASSERT_EQ(mServer.CreateActiveSession(sessionId), CHIP_ERROR_INVALID_ARGUMENT);

    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    ASSERT_FALSE(mClusterTester.GetNextGeneratedEvent().has_value());
}

TEST_F(TestRemoteAvAnalysisCluster, CreateActiveSessionRecordsASessionWithoutAStartEvent)
{
    constexpr NodeId kSourceCamera        = 0xCA11;
    constexpr uint64_t kStreamStartUs     = 987654321;
    ServerClusterContext & clusterContext = mClusterTester.GetServerClusterContext();

    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, kSourceCamera, Status::Success, 7);

    InvalidatableCommandHandler enableHandler;
    enableHandler.SetFabricIndex(1);
    ConcreteCommandPath enablePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType enableData;
    enableData.contextTriggers.SetNull();
    ASSERT_TRUE(StatusOf(mServer.GetLogic().HandleEnableContextTriggers(enableHandler, enablePath, enableData)).IsSuccess());

    // The point of the API: the session exists, but no AnalysisSessionStart is reported for it
    uint16_t sessionId = 0;
    ASSERT_EQ(mServer.CreateActiveSession(sessionId, kSourceCamera, kStreamStartUs), CHIP_NO_ERROR);
    ASSERT_FALSE(mClusterTester.GetNextGeneratedEvent().has_value());

    // And the source it was given reaches the events of that session
    const std::vector<Structs::TrackedContext::Type> trackedContext = {
        { .identifiedContextID = 0,
          .identifiedContext   = { .namespaceID = static_cast<uint8_t>(0x49), .tag = static_cast<uint8_t>(0x0B) },
          .startTime           = 0,
          .endTime             = DataModel::NullNullable }
    };
    ASSERT_EQ(mServer.GetLogic().InitialTriggeringContextDetected(sessionId, trackedContext, &clusterContext), CHIP_NO_ERROR);
    auto perceivedEvent = mClusterTester.GetNextGeneratedEvent();
    ASSERT_TRUE(perceivedEvent.has_value());
    Events::PerceivedContext::DecodableType perceivedData;
    if (perceivedEvent.has_value())
    {
        ASSERT_EQ(perceivedEvent->GetEventData(perceivedData), CHIP_NO_ERROR);
    }
    ASSERT_EQ(perceivedData.sessionID, sessionId);
    ASSERT_TRUE(perceivedData.sourceNodeId.HasValue());
    ASSERT_EQ(perceivedData.sourceNodeId.Value(), kSourceCamera);
    ASSERT_TRUE(perceivedData.sourceStartTimestamp.HasValue());
    ASSERT_EQ(perceivedData.sourceStartTimestamp.Value(), kStreamStartUs);
}

TEST_F(TestRemoteAvAnalysisCluster, SessionIdsFromBothPathsDoNotCollide)
{
    constexpr NodeId kSourceCamera    = 0xCA11;
    constexpr uint64_t kStreamStartUs = 555;

    // A session recorded under an id of the caller's choosing does not move the counter that
    // AnalysisSessionStart assigns from, so that id has to be skipped rather than handed out again
    uint16_t chosenId = 3;
    ASSERT_EQ(mServer.CreateActiveSession(chosenId, kSourceCamera, kStreamStartUs, /* aUseSpecificSessionId = */ true),
              CHIP_NO_ERROR);
    ASSERT_EQ(chosenId, 3);

    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, kSourceCamera, Status::Success, 7);

    // Start sessions until the counter has passed the chosen id; none may reuse it
    for (int i = 0; i < 5; i++)
    {
        uint16_t sessionId = 0;
        ASSERT_EQ(mServer.AnalysisSessionStart(sessionId, DataModel::NullNullable, kSourceCamera, kStreamStartUs), CHIP_NO_ERROR);
        ASSERT_NE(sessionId, chosenId);
        ASSERT_TRUE(mClusterTester.GetNextGeneratedEvent().has_value());
    }
}

TEST_F(TestRemoteAvAnalysisCluster, EventsCarryTheSourceCameraOfTheSession)
{
    constexpr NodeId kSourceCamera        = 0xCA11;
    constexpr uint64_t kStreamStartUs     = 123456789;
    ServerClusterContext & clusterContext = mClusterTester.GetServerClusterContext();

    // Arm all triggers (requires an established stream first)
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, kSourceCamera, Status::Success, 7);
    InvalidatableCommandHandler enableHandler;
    enableHandler.SetFabricIndex(1);
    ConcreteCommandPath enablePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType enableData;
    enableData.contextTriggers.SetNull();
    auto enableResponse = mServer.GetLogic().HandleEnableContextTriggers(enableHandler, enablePath, enableData);
    ASSERT_TRUE(enableResponse.has_value() && enableResponse.value().IsSuccess());

    uint16_t sessionId = 0;
    ASSERT_EQ(mServer.AnalysisSessionStart(sessionId, DataModel::NullNullable, kSourceCamera, kStreamStartUs), CHIP_NO_ERROR);
    auto startEvent = mClusterTester.GetNextGeneratedEvent();
    ASSERT_TRUE(startEvent.has_value());
    Events::AnalysisSessionStart::DecodableType startData;
    if (startEvent.has_value()) // the ASSERT above is not a guard for clang-tidy's optional check
    {
        ASSERT_EQ(startEvent->GetEventData(startData), CHIP_NO_ERROR);
    }
    ASSERT_TRUE(startData.sourceNodeId.HasValue());
    ASSERT_EQ(startData.sourceNodeId.Value(), kSourceCamera);

    // PerceivedContext: same camera, plus the stream's start timestamp
    const std::vector<Structs::TrackedContext::Type> trackedContext = {
        { .identifiedContextID = 0,
          .identifiedContext   = { .namespaceID = static_cast<uint8_t>(0x49), .tag = static_cast<uint8_t>(0x0B) },
          .startTime           = 0,
          .endTime             = DataModel::NullNullable }
    };
    ASSERT_EQ(mServer.GetLogic().InitialTriggeringContextDetected(sessionId, trackedContext, &clusterContext), CHIP_NO_ERROR);
    auto perceivedEvent = mClusterTester.GetNextGeneratedEvent();
    ASSERT_TRUE(perceivedEvent.has_value());
    Events::PerceivedContext::DecodableType perceivedData;
    if (perceivedEvent.has_value())
    {
        ASSERT_EQ(perceivedEvent->GetEventData(perceivedData), CHIP_NO_ERROR);
    }
    ASSERT_TRUE(perceivedData.sourceNodeId.HasValue());
    ASSERT_EQ(perceivedData.sourceNodeId.Value(), kSourceCamera);
    ASSERT_TRUE(perceivedData.sourceStartTimestamp.HasValue());
    ASSERT_EQ(perceivedData.sourceStartTimestamp.Value(), kStreamStartUs);

    // The later PerceivedContext paths carry the same source fields (a second instance of the one
    // enabled context; NewContextDetected only accepts contexts among the active triggers)
    const std::vector<Structs::TrackedContext::Type> additionalContext = {
        { .identifiedContextID = 1,
          .identifiedContext   = { .namespaceID = static_cast<uint8_t>(0x49), .tag = static_cast<uint8_t>(0x0B) },
          .startTime           = 0,
          .endTime             = DataModel::NullNullable }
    };
    ASSERT_EQ(mServer.GetLogic().NewContextDetected(sessionId, additionalContext, &clusterContext), CHIP_NO_ERROR);
    auto newContextEvent = mClusterTester.GetNextGeneratedEvent();
    ASSERT_TRUE(newContextEvent.has_value());
    Events::PerceivedContext::DecodableType newContextData;
    if (newContextEvent.has_value())
    {
        ASSERT_EQ(newContextEvent->GetEventData(newContextData), CHIP_NO_ERROR);
    }
    ASSERT_TRUE(newContextData.sourceNodeId.HasValue());
    ASSERT_EQ(newContextData.sourceNodeId.Value(), kSourceCamera);
    ASSERT_TRUE(newContextData.sourceStartTimestamp.HasValue());
    ASSERT_EQ(newContextData.sourceStartTimestamp.Value(), kStreamStartUs);

    ASSERT_EQ(mServer.GetLogic().ContextNoLongerDetected(sessionId, trackedContext, &clusterContext), CHIP_NO_ERROR);
    auto expiredContextEvent = mClusterTester.GetNextGeneratedEvent();
    ASSERT_TRUE(expiredContextEvent.has_value());
    Events::PerceivedContext::DecodableType expiredContextData;
    if (expiredContextEvent.has_value())
    {
        ASSERT_EQ(expiredContextEvent->GetEventData(expiredContextData), CHIP_NO_ERROR);
    }
    ASSERT_TRUE(expiredContextData.sourceNodeId.HasValue());
    ASSERT_EQ(expiredContextData.sourceNodeId.Value(), kSourceCamera);
    ASSERT_TRUE(expiredContextData.sourceStartTimestamp.HasValue());
    ASSERT_EQ(expiredContextData.sourceStartTimestamp.Value(), kStreamStartUs);

    // Session end: the source matches the associated AnalysisSessionStart
    ASSERT_EQ(mServer.GetLogic().AnalysisSessionEnd(sessionId, &clusterContext), CHIP_NO_ERROR);
    auto endEvent = mClusterTester.GetNextGeneratedEvent();
    ASSERT_TRUE(endEvent.has_value());
    Events::AnalysisSessionEnd::DecodableType endData;
    if (endEvent.has_value())
    {
        ASSERT_EQ(endEvent->GetEventData(endData), CHIP_NO_ERROR);
    }
    ASSERT_TRUE(endData.sourceNodeId.HasValue());
    ASSERT_EQ(endData.sourceNodeId.Value(), kSourceCamera);
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
    size_t triggersSize = 1;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, aActiveContextTriggers), CHIP_NO_ERROR);
    ASSERT_EQ(aActiveContextTriggers.ComputeSize(&triggersSize), CHIP_NO_ERROR);
    ASSERT_EQ(triggersSize, static_cast<size_t>(0));

    uint8_t aMaxAnalysisStreamCount = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::MaxAnalysisStreamCount::Id, aMaxAnalysisStreamCount), CHIP_NO_ERROR);
    ASSERT_EQ(kTestMaxAnalysisStreams, aMaxAnalysisStreamCount);

    uint8_t aCurrentAnalysisStreamCount = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, aCurrentAnalysisStreamCount), CHIP_NO_ERROR);
    ASSERT_EQ(0, aCurrentAnalysisStreamCount);

    // On startup there should be no analysis streams
    Attributes::AnalysisStreams::TypeInfo::DecodableType aAnalysisStreams;
    size_t streamsSize = 1;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, aAnalysisStreams), CHIP_NO_ERROR);
    ASSERT_EQ(aAnalysisStreams.ComputeSize(&streamsSize), CHIP_NO_ERROR);
    ASSERT_EQ(streamsSize, static_cast<size_t>(0));

    bool trackingEnabled = false;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::TrackingEnabled::Id, trackingEnabled), CHIP_NO_ERROR);
    ASSERT_FALSE(trackingEnabled);
}

TEST_F(TestRemoteAvAnalysisCluster, ExecuteDisableContextTriggersCommandTest)
{
    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::DisableContextTriggers::Id };
    Commands::DisableContextTriggers::DecodableType commandData;

    auto response = mServer.GetLogic().HandleDisableContextTriggers(commandHandler, kCommandPath, commandData);

    // The response should contain an ActionReturnStatus
    if (response.has_value())
    {
        ASSERT_TRUE(StatusOf(response).IsSuccess());
    }
    else
    {
        FAIL();
    }
}

TEST_F(TestRemoteAvAnalysisCluster, EstablishAnalysisStreamSuccess)
{
    constexpr NodeId kCameraNodeId = 0x1234;
    InvalidatableCommandHandler commandHandler;
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
    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);

    EstablishStream(commandHandler, 0x1234, Status::ResourceExhausted, 0);

    ASSERT_FALSE(commandHandler.HasResponse());
    ASSERT_TRUE(commandHandler.HasStatus());
    ASSERT_EQ(LastStatus(commandHandler), Status::ResourceExhausted);

    uint8_t currentCount = 0xFF;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 0);
}

TEST_F(TestRemoteAvAnalysisCluster, EstablishAnalysisStreamBusyWhileAllocationPending)
{
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
    Commands::EstablishAnalysisStream::DecodableType commandData;
    commandData.nodeID = 0x1234;

    InvalidatableCommandHandler firstHandler;
    firstHandler.SetFabricIndex(1);
    auto firstResponse = mServer.GetLogic().HandleEstablishAnalysisStream(firstHandler, path, commandData);
    ASSERT_FALSE(firstResponse.has_value()); // Pending on the camera

    // A second camera-bound command while the first is pending is answered with Busy
    InvalidatableCommandHandler secondHandler;
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
        InvalidatableCommandHandler commandHandler;
        commandHandler.SetFabricIndex(1);
        EstablishStream(commandHandler, 0x1234, Status::Success, id);
        ASSERT_TRUE(commandHandler.HasResponse());
    }

    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
    Commands::EstablishAnalysisStream::DecodableType commandData;
    commandData.nodeID = 0x1234;
    InvalidatableCommandHandler commandHandler;
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

TEST_F(TestRemoteAvAnalysisCluster, EstablishThatCannotStartIsAnsweredAtOnce)
{
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
    Commands::EstablishAnalysisStream::DecodableType commandData;
    commandData.nodeID = 0x1234;

    // The client is busy with a request of its own
    mFakeCameraClient.mAllocationResult = CHIP_ERROR_BUSY;
    InvalidatableCommandHandler busyHandler;
    busyHandler.SetFabricIndex(1);
    auto busyResponse = mServer.GetLogic().HandleEstablishAnalysisStream(busyHandler, path, commandData);
    ASSERT_TRUE(busyResponse.has_value()) << "EstablishAnalysisStream unexpectedly parked";
    ASSERT_EQ(StatusOf(busyResponse).GetStatusCode().GetStatus(), Status::Busy);

    // The client refuses for any other reason
    mFakeCameraClient.mAllocationResult = CHIP_ERROR_NO_MEMORY;
    InvalidatableCommandHandler failHandler;
    failHandler.SetFabricIndex(1);
    auto failResponse = mServer.GetLogic().HandleEstablishAnalysisStream(failHandler, path, commandData);
    ASSERT_TRUE(failResponse.has_value()) << "EstablishAnalysisStream unexpectedly parked";
    ASSERT_EQ(StatusOf(failResponse).GetStatusCode().GetStatus(), Status::Failure);

    // Neither refusal left an interaction behind
    mFakeCameraClient.mAllocationResult = CHIP_NO_ERROR;
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ASSERT_EQ(mFakeCameraClient.mAllocationRequests, 1);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kPendingInitiation);
}

TEST_F(TestRemoteAvAnalysisCluster, RemoveThatCannotStartLeavesTheStream)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    // The client is busy with a request of its own
    mFakeCameraClient.mDeallocationResult = CHIP_ERROR_BUSY;
    ASSERT_EQ(ImmediateRemoveStatus(0), Status::Busy);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kPendingInitiation);

    // The client refuses for any other reason
    mFakeCameraClient.mDeallocationResult = CHIP_ERROR_NO_MEMORY;
    ASSERT_EQ(ImmediateRemoveStatus(0), Status::Failure);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kPendingInitiation);

    // Neither refusal left an interaction behind
    mFakeCameraClient.mDeallocationResult = CHIP_NO_ERROR;
    InvalidatableCommandHandler removeHandler;
    removeHandler.SetFabricIndex(1);
    ConcreteCommandPath removePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType removeData;
    removeData.analysisStreamID = 0;
    ASSERT_FALSE(mServer.GetLogic().HandleRemoveAnalysisStream(removeHandler, removePath, removeData).has_value());
    ASSERT_NE(mFakeCameraClient.mLastCallback, nullptr);
    mFakeCameraClient.mLastCallback->OnVideoStreamDeallocated(Status::Success, 42);
    ASSERT_EQ(LastStatus(removeHandler), Status::Success);
}

TEST_F(TestRemoteAvAnalysisCluster, ExecuteActivateAnalysisStreamCommandTest)
{
    InvalidatableCommandHandler commandHandler;
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

    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 0);
}

TEST_F(TestRemoteAvAnalysisCluster, ActivateRequiresExactlyOneEndpointField)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
    Commands::ActivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;

    // Neither endpoint field selects a transport
    auto response = mServer.GetLogic().HandleActivateAnalysisStream(commandHandler, path, commandData);
    ASSERT_TRUE(response.has_value());
    ASSERT_EQ(StatusOf(response).GetStatusCode().GetStatus(), Status::InvalidCommand);

    // Both fields are ambiguous
    commandData.webRTCEndpointID = MakeOptional(static_cast<EndpointId>(2));
    commandData.pushAVEndpointID = MakeOptional(static_cast<EndpointId>(3));
    response                     = mServer.GetLogic().HandleActivateAnalysisStream(commandHandler, path, commandData);
    ASSERT_TRUE(response.has_value());
    ASSERT_EQ(StatusOf(response).GetStatusCode().GetStatus(), Status::InvalidCommand);

    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 0);
}

TEST_F(TestRemoteAvAnalysisCluster, EveryCommandReachesItsOwnHandler)
{
    // One stream in PendingInitiation, so every command has something to act on
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    // Each command is given arguments only its own handler answers this way, so a request reaching
    // the wrong one changes the status. The six answers are pairwise distinct.

    // EnableContextTriggers: a context outside SupportedAmbientContexts
    Structs::ContextTriggerStruct::Type unsupported;
    unsupported.context.namespaceID = 0x99;
    unsupported.context.tag         = 0x99;
    Commands::EnableContextTriggers::Type enable;
    enable.contextTriggers.SetNonNull(DataModel::List<const Structs::ContextTriggerStruct::Type>(&unsupported, 1));
    EXPECT_EQ(InvokedStatus(Commands::EnableContextTriggers::Id, enable), Status::ConstraintError);

    // DisableContextTriggers: a null list disables everything and succeeds
    Commands::DisableContextTriggers::Type disable;
    disable.contextTriggers.SetNull();
    EXPECT_EQ(InvokedStatus(Commands::DisableContextTriggers::Id, disable), Status::Success);

    // EstablishAnalysisStream: the camera client is busy, so it answers without parking
    mFakeCameraClient.mAllocationResult = CHIP_ERROR_BUSY;
    Commands::EstablishAnalysisStream::Type establish;
    establish.nodeID = 0x1234;
    EXPECT_EQ(InvokedStatus(Commands::EstablishAnalysisStream::Id, establish), Status::Busy);

    // ActivateAnalysisStream: both endpoint fields, the only command with any
    Commands::ActivateAnalysisStream::Type activate;
    activate.analysisStreamID = 0;
    activate.webRTCEndpointID = MakeOptional(static_cast<EndpointId>(2));
    activate.pushAVEndpointID = MakeOptional(static_cast<EndpointId>(3));
    EXPECT_EQ(InvokedStatus(Commands::ActivateAnalysisStream::Id, activate), Status::InvalidCommand);

    // DeactivateAnalysisStream: the stream is not active
    Commands::DeactivateAnalysisStream::Type deactivate;
    deactivate.analysisStreamID = 0;
    EXPECT_EQ(InvokedStatus(Commands::DeactivateAnalysisStream::Id, deactivate), Status::InvalidInState);

    // RemoveAnalysisStream: removable, but the camera client refuses outright
    mFakeCameraClient.mDeallocationResult = CHIP_ERROR_NO_MEMORY;
    Commands::RemoveAnalysisStream::Type remove;
    remove.analysisStreamID = 0;
    EXPECT_EQ(InvokedStatus(Commands::RemoveAnalysisStream::Id, remove), Status::Failure);
}

TEST_F(TestRemoteAvAnalysisCluster, ActivateEndpointFieldRuleHoldsOverTheWire)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    // Exactly one endpoint field selects the transport, decoded from the request rather than
    // assembled in the test: absence and presence are what the encoding carries
    Commands::ActivateAnalysisStream::Type neither;
    neither.analysisStreamID = 0;
    EXPECT_EQ(InvokedStatus(Commands::ActivateAnalysisStream::Id, neither), Status::InvalidCommand);

    Commands::ActivateAnalysisStream::Type both;
    both.analysisStreamID = 0;
    both.webRTCEndpointID = MakeOptional(static_cast<EndpointId>(2));
    both.pushAVEndpointID = MakeOptional(static_cast<EndpointId>(3));
    EXPECT_EQ(InvokedStatus(Commands::ActivateAnalysisStream::Id, both), Status::InvalidCommand);

    // PushAV alone is well formed, and unsupported by this implementation
    Commands::ActivateAnalysisStream::Type pushAVOnly;
    pushAVOnly.analysisStreamID = 0;
    pushAVOnly.pushAVEndpointID = MakeOptional(static_cast<EndpointId>(3));
    EXPECT_EQ(InvokedStatus(Commands::ActivateAnalysisStream::Id, pushAVOnly), Status::InvalidCommand);

    // WebRTC alone is accepted: it parks on the offer exchange rather than answering
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kPendingInitiation);
    ActivateStream(0, 2, 55);
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 1);
}

TEST_F(TestRemoteAvAnalysisCluster, ActivateWithANonEndpointIsAConstraintError)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    // The endpoint is encoded into AnalysisStreams, where an out-of-range one fails every read
    ASSERT_EQ(ImmediateActivateStatus(0, kInvalidEndpointId), Status::ConstraintError);
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 0);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kPendingInitiation);
}

TEST_F(TestRemoteAvAnalysisCluster, DestructionAnswersACommandParkedOnACameraInteraction)
{
    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
    Commands::EstablishAnalysisStream::DecodableType commandData;
    commandData.nodeID = 0x1234;

    {
        AvAnalysisCluster cluster(kTestEndpointId, BitFlags<Feature>(Feature::kRemoteContextDetection), testAmbientContexts,
                                  DataModel::NullNullable, kTestMaxAnalysisStreams);
        cluster.SetDelegate(&mMockDelegate);
        cluster.SetCameraClient(&mFakeCameraClient);
        cluster.SetWebRTCClient(&mFakeWebRTCClient);
        ASSERT_EQ(cluster.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);

        // Parked: the response waits on an allocation the camera will never answer
        ASSERT_FALSE(cluster.GetLogic().HandleEstablishAnalysisStream(commandHandler, path, commandData).has_value());
        ASSERT_FALSE(commandHandler.HasStatus());

        // The instance goes away without a Shutdown of its own
    }

    // Abandoning the interaction cannot complete the command, so it is failed rather than dropped
    ASSERT_EQ(LastStatus(commandHandler), Status::Failure);
}

TEST_F(TestRemoteAvAnalysisCluster, ActivateWithPushAVEndpointIsUnsupported)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
    Commands::ActivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    commandData.pushAVEndpointID = MakeOptional(static_cast<EndpointId>(3));

    auto response = mServer.GetLogic().HandleActivateAnalysisStream(commandHandler, path, commandData);
    ASSERT_TRUE(response.has_value());
    ASSERT_EQ(StatusOf(response).GetStatusCode().GetStatus(), Status::InvalidCommand);
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 0);
}

TEST_F(TestRemoteAvAnalysisCluster, ActivateInitiatesAWebRTCSession)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    InvalidatableCommandHandler activateHandler;
    activateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
    Commands::ActivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    commandData.webRTCEndpointID = MakeOptional(static_cast<EndpointId>(2));

    auto response = mServer.GetLogic().HandleActivateAnalysisStream(activateHandler, path, commandData);
    ASSERT_FALSE(response.has_value()); // Response is pending on the offer exchange

    // The session request carries the camera, the provided endpoint, and the camera's VideoStreamID
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 1);
    ASSERT_EQ(mFakeWebRTCClient.mLastCamera, ScopedNodeId(0x1234, 1));
    ASSERT_EQ(mFakeWebRTCClient.mLastEndpoint, 2);
    ASSERT_EQ(mFakeWebRTCClient.mLastVideoStream, 42);

    // A concurrent camera-bound command answers Busy while the offer exchange is in flight
    InvalidatableCommandHandler busyHandler;
    busyHandler.SetFabricIndex(1);
    Commands::RemoveAnalysisStream::DecodableType removeData;
    removeData.analysisStreamID = 0;
    ConcreteCommandPath removePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    auto busyResponse = mServer.GetLogic().HandleRemoveAnalysisStream(busyHandler, removePath, removeData);
    ASSERT_TRUE(busyResponse.has_value());
    ASSERT_EQ(StatusOf(busyResponse).GetStatusCode().GetStatus(), Status::Busy);

    // The camera assigns session 55: the entry records it and the command answers SUCCESS
    ASSERT_NE(mFakeWebRTCClient.mLastCallback, nullptr);
    mFakeWebRTCClient.mLastCallback->OnSessionInitiated(Status::Success, 55, /* aOfferSent = */ true);
    ASSERT_EQ(LastStatus(activateHandler), Status::Success);

    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kWebRTCInitiated);
    ASSERT_TRUE(iter.GetValue().webRTCEndpointID.HasValue());
    ASSERT_FALSE(iter.GetValue().webRTCEndpointID.Value().IsNull());
    ASSERT_EQ(iter.GetValue().webRTCEndpointID.Value().Value(), 2);

    // A second Activate on the initiated stream is a SUCCESS no-op
    InvalidatableCommandHandler secondHandler;
    secondHandler.SetFabricIndex(1);
    auto secondResponse = mServer.GetLogic().HandleActivateAnalysisStream(secondHandler, path, commandData);
    ASSERT_TRUE(secondResponse.has_value());
    ASSERT_TRUE(StatusOf(secondResponse).IsSuccess());
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 1);
}

TEST_F(TestRemoteAvAnalysisCluster, ActivationFailingAfterTheOfferMarksTheStreamFailed)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    InvalidatableCommandHandler activateHandler;
    activateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
    Commands::ActivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    commandData.webRTCEndpointID = MakeOptional(static_cast<EndpointId>(2));

    auto response = mServer.GetLogic().HandleActivateAnalysisStream(activateHandler, path, commandData);
    ASSERT_FALSE(response.has_value());

    // The camera refused the offer it received: the flow failed after initiating, so the stream is
    // Failure rather than the PendingInitiation it started from (11.9.8.5)
    ASSERT_NE(mFakeWebRTCClient.mLastCallback, nullptr);
    mFakeWebRTCClient.mLastCallback->OnSessionInitiated(Status::InvalidInState, 0, /* aOfferSent = */ true);
    ASSERT_EQ(LastStatus(activateHandler), Status::InvalidInState);
    ASSERT_EQ(StreamState(0), AnalysisStreamStateEnum::kFailure);

    // A failed stream is activatable again: the command parks on a fresh request to the camera
    InvalidatableCommandHandler retryHandler;
    retryHandler.SetFabricIndex(1);
    auto retry = mServer.GetLogic().HandleActivateAnalysisStream(retryHandler, path, commandData);
    ASSERT_FALSE(retry.has_value());
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 2);
}

TEST_F(TestRemoteAvAnalysisCluster, ActivateInitiationFailureIsPropagatedWithoutSideEffects)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    InvalidatableCommandHandler activateHandler;
    activateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
    Commands::ActivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    commandData.webRTCEndpointID = MakeOptional(static_cast<EndpointId>(2));

    auto response = mServer.GetLogic().HandleActivateAnalysisStream(activateHandler, path, commandData);
    ASSERT_FALSE(response.has_value());

    // No WebRTCTransportProvider on the camera surfaces as NOT_FOUND, before any offer is sent
    ASSERT_NE(mFakeWebRTCClient.mLastCallback, nullptr);
    mFakeWebRTCClient.mLastCallback->OnSessionInitiated(Status::NotFound, 0, /* aOfferSent = */ false);
    ASSERT_EQ(LastStatus(activateHandler), Status::NotFound);

    // The entry is untouched: still PendingInitiation, no endpoint, and activatable again
    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kPendingInitiation);
    ASSERT_TRUE(iter.GetValue().webRTCEndpointID.HasValue());
    ASSERT_TRUE(iter.GetValue().webRTCEndpointID.Value().IsNull());
    // No session was recorded either: there is nothing to deactivate
    ASSERT_EQ(ImmediateDeactivateStatus(0), Status::InvalidInState);

    InvalidatableCommandHandler retryHandler;
    retryHandler.SetFabricIndex(1);
    auto retryResponse = mServer.GetLogic().HandleActivateAnalysisStream(retryHandler, path, commandData);
    ASSERT_FALSE(retryResponse.has_value());
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 2);

    // Settle the retry: a mock handler must not outlive the test with an interaction parked on it
    mFakeWebRTCClient.mLastCallback->OnSessionInitiated(Status::Success, 56, /* aOfferSent = */ true);
    ASSERT_EQ(LastStatus(retryHandler), Status::Success);
}

TEST_F(TestRemoteAvAnalysisCluster, ActivateThatCannotStartIsAnsweredAtOnce)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    // The client is busy with a send of its own: Busy, nothing parked
    mFakeWebRTCClient.mRequestSessionResult = CHIP_ERROR_BUSY;
    ASSERT_EQ(ImmediateActivateStatus(0, 2), Status::Busy);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kPendingInitiation);

    // The client refuses for any other reason: Failure, nothing parked
    mFakeWebRTCClient.mRequestSessionResult = CHIP_ERROR_NO_MEMORY;
    ASSERT_EQ(ImmediateActivateStatus(0, 2), Status::Failure);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kPendingInitiation);

    // Neither refusal left an interaction behind: the next Activate is taken
    mFakeWebRTCClient.mRequestSessionResult = CHIP_NO_ERROR;
    ActivateStream(0, 2, 55);
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 3);
}

TEST_F(TestRemoteAvAnalysisCluster, SessionActiveMarksTheStreamActive)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);

    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kWebRTCActive);

    // An unknown session going active changes nothing, and reports nothing changed
    const size_t dirtyBefore = mClusterTester.GetDirtyList().size();
    mFakeWebRTCClient.mLastCallback->OnSessionActive(ScopedNodeId(0x1234, 1), 99);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kWebRTCActive);
    ASSERT_EQ(mClusterTester.GetDirtyList().size(), dirtyBefore);
}

TEST_F(TestRemoteAvAnalysisCluster, SessionsOfDifferentCamerasSharingAnIdAreToldApart)
{
    // Two cameras, each assigning WebRTC session id 0 to its stream
    InvalidatableCommandHandler firstHandler;
    firstHandler.SetFabricIndex(1);
    EstablishStream(firstHandler, 0x1234, Status::Success, 42);
    InvalidatableCommandHandler secondHandler;
    secondHandler.SetFabricIndex(1);
    EstablishStream(secondHandler, 0x5678, Status::Success, 42);
    ActivateStream(0, 2, 0, ScopedNodeId(0x1234, 1));
    ActivateStream(1, 2, 0, ScopedNodeId(0x5678, 1));

    // The second camera's session fails: only its stream does
    mFakeWebRTCClient.mLastCallback->OnSessionFailed(ScopedNodeId(0x5678, 1), 0);

    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamID, 0);
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kWebRTCActive);
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamID, 1);
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kFailure);
}

TEST_F(TestRemoteAvAnalysisCluster, SessionFailureMarksTheStreamFailed)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);

    // The flow failing at any point post-initiation marks the stream Failure.
    // The dead session's id is forgotten; the endpoint stays until deactivation or re-activation.
    mFakeWebRTCClient.mLastCallback->OnSessionFailed(ScopedNodeId(0x1234, 1), 55);

    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kFailure);
    ASSERT_TRUE(iter.GetValue().webRTCEndpointID.HasValue());
    ASSERT_FALSE(iter.GetValue().webRTCEndpointID.Value().IsNull());
    ASSERT_EQ(iter.GetValue().webRTCEndpointID.Value().Value(), 2);

    // A repeated report for the dead session changes nothing, and reports nothing changed
    const size_t dirtyBefore = mClusterTester.GetDirtyList().size();
    mFakeWebRTCClient.mLastCallback->OnSessionFailed(ScopedNodeId(0x1234, 1), 55);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kFailure);
    ASSERT_EQ(mClusterTester.GetDirtyList().size(), dirtyBefore);

    // Failure is an activatable state: the stream can be re-initiated
    ActivateStream(0, 2, 77);
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 2);
}

TEST_F(TestRemoteAvAnalysisCluster, SessionFailureBeforeActiveMarksTheStreamFailed)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    // Initiated but not yet active: the answer/ICE flow collapses
    InvalidatableCommandHandler activateHandler;
    activateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
    Commands::ActivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    commandData.webRTCEndpointID = MakeOptional(static_cast<EndpointId>(2));
    auto response                = mServer.GetLogic().HandleActivateAnalysisStream(activateHandler, path, commandData);
    ASSERT_FALSE(response.has_value());
    mFakeWebRTCClient.mLastCallback->OnSessionInitiated(Status::Success, 55, /* aOfferSent = */ true);
    ASSERT_EQ(LastStatus(activateHandler), Status::Success);

    mFakeWebRTCClient.mLastCallback->OnSessionFailed(ScopedNodeId(0x1234, 1), 55);

    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kFailure);
}

TEST_F(TestRemoteAvAnalysisCluster, DeactivateEndsTheSession)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);

    InvalidatableCommandHandler deactivateHandler;
    deactivateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;

    auto response = mServer.GetLogic().HandleDeactivateAnalysisStream(deactivateHandler, path, commandData);
    ASSERT_FALSE(response.has_value()); // Response is pending on the camera's EndSession answer

    // once EndSession is sent the stream is WebRTCPendingDeactivation
    Attributes::AnalysisStreams::TypeInfo::DecodableType pendingStreams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, pendingStreams), CHIP_NO_ERROR);
    auto pendingIter = pendingStreams.begin();
    ASSERT_TRUE(pendingIter.Next());
    ASSERT_EQ(pendingIter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kWebRTCPendingDeactivation);

    // EndSession names the camera, the stored endpoint, and the stored session id
    ASSERT_EQ(mFakeWebRTCClient.mEndRequests, 1);
    ASSERT_EQ(mFakeWebRTCClient.mLastCamera, ScopedNodeId(0x1234, 1));
    ASSERT_EQ(mFakeWebRTCClient.mLastEndpoint, 2);
    ASSERT_EQ(mFakeWebRTCClient.mLastSessionId, 55);

    // The camera confirms: back to an activatable stream with no session associations
    mFakeWebRTCClient.mLastCallback->OnSessionEnded(Status::Success, 55);
    ASSERT_EQ(LastStatus(deactivateHandler), Status::Success);

    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kPendingInitiation);
    ASSERT_TRUE(iter.GetValue().webRTCEndpointID.HasValue());
    ASSERT_TRUE(iter.GetValue().webRTCEndpointID.Value().IsNull());

    // The stream is activatable again
    ActivateStream(0, 2, 77);
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 2);
}

TEST_F(TestRemoteAvAnalysisCluster, DeactivateThatCannotStartLeavesTheStreamActive)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);

    // The client cannot take the request: nothing is sent
    mFakeWebRTCClient.mEndSessionResult = CHIP_ERROR_BUSY;

    InvalidatableCommandHandler deactivateHandler;
    deactivateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;

    auto response = mServer.GetLogic().HandleDeactivateAnalysisStream(deactivateHandler, path, commandData);
    ASSERT_TRUE(response.has_value());
    ASSERT_EQ(StatusOf(response).GetStatusCode().GetStatus(), Status::Busy);

    // The session is untouched, so the stream is still active and deactivatable
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kWebRTCActive);

    // A refusal for any other reason is a Failure, with the stream still active
    mFakeWebRTCClient.mEndSessionResult = CHIP_ERROR_NO_MEMORY;
    ASSERT_EQ(ImmediateDeactivateStatus(0), Status::Failure);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kWebRTCActive);

    mFakeWebRTCClient.mEndSessionResult = CHIP_NO_ERROR;
    InvalidatableCommandHandler retryHandler;
    retryHandler.SetFabricIndex(1);
    response = mServer.GetLogic().HandleDeactivateAnalysisStream(retryHandler, path, commandData);
    ASSERT_FALSE(response.has_value());
    ASSERT_EQ(mFakeWebRTCClient.mEndRequests, 3);

    // Finish the retry so no interaction outlives its handler
    mFakeWebRTCClient.mLastCallback->OnSessionEnded(Status::Success, 55);
    ASSERT_EQ(LastStatus(retryHandler), Status::Success);
}

TEST_F(TestRemoteAvAnalysisCluster, DeactivateCompletingSynchronouslyKeepsItsOutcome)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);

    // The client fails the request before EndSession() returns (a cached CASE session whose send
    // fails at once); the handler must not overwrite that outcome afterwards
    mFakeWebRTCClient.mSynchronousEndOutcome = Status::Failure;

    InvalidatableCommandHandler deactivateHandler;
    deactivateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;

    auto response = mServer.GetLogic().HandleDeactivateAnalysisStream(deactivateHandler, path, commandData);
    ASSERT_FALSE(response.has_value()); // The parked handler already answered
    ASSERT_EQ(LastStatus(deactivateHandler), Status::Failure);

    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kFailure);

    // Failure is an exit: the stream can be re-initiated
    mFakeWebRTCClient.mSynchronousEndOutcome.reset();
    ActivateStream(0, 2, 77);
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 2);
}

TEST_F(TestRemoteAvAnalysisCluster, DeactivateFailureMarksTheStreamFailed)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);

    InvalidatableCommandHandler deactivateHandler;
    deactivateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;

    auto response = mServer.GetLogic().HandleDeactivateAnalysisStream(deactivateHandler, path, commandData);
    ASSERT_FALSE(response.has_value());

    // The camera's failure is propagated and the stream is marked Failure; the endpoint stays populated
    mFakeWebRTCClient.mLastCallback->OnSessionEnded(Status::NotFound, 55);
    ASSERT_EQ(LastStatus(deactivateHandler), Status::NotFound);

    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kFailure);
    ASSERT_TRUE(iter.GetValue().webRTCEndpointID.HasValue());
    ASSERT_FALSE(iter.GetValue().webRTCEndpointID.Value().IsNull());
    ASSERT_EQ(iter.GetValue().webRTCEndpointID.Value().Value(), 2);

    // The client is done with session 55, so the stream no longer refers to it: late signals for
    // it change nothing, and report nothing changed
    const size_t dirtyBefore = mClusterTester.GetDirtyList().size();
    mFakeWebRTCClient.mLastCallback->OnSessionActive(ScopedNodeId(0x1234, 1), 55);
    mFakeWebRTCClient.mLastCallback->OnSessionFailed(ScopedNodeId(0x1234, 1), 55);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kFailure);
    ASSERT_EQ(mClusterTester.GetDirtyList().size(), dirtyBefore);

    // Failure is an activatable state: the stream can be re-initiated
    ActivateStream(0, 2, 77);
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 2);
}

TEST_F(TestRemoteAvAnalysisCluster, DeactivateIsRejectedUnlessTheStreamIsActive)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    // Initiated: the offer exchange completed, the connection has not
    InvalidatableCommandHandler activateHandler;
    activateHandler.SetFabricIndex(1);
    ConcreteCommandPath activatePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
    Commands::ActivateAnalysisStream::DecodableType activateData;
    activateData.analysisStreamID = 0;
    activateData.webRTCEndpointID = MakeOptional(static_cast<EndpointId>(2));
    ASSERT_FALSE(mServer.GetLogic().HandleActivateAnalysisStream(activateHandler, activatePath, activateData).has_value());
    mFakeWebRTCClient.mLastCallback->OnSessionInitiated(Status::Success, 55, /* aOfferSent = */ true);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kWebRTCInitiated);
    ASSERT_EQ(ImmediateDeactivateStatus(0), Status::InvalidInState);

    // Active: accepted, and a second Deactivate while the first is pending is rejected
    mFakeWebRTCClient.mLastCallback->OnSessionActive(ScopedNodeId(0x1234, 1), 55);
    InvalidatableCommandHandler deactivateHandler;
    deactivateHandler.SetFabricIndex(1);
    ConcreteCommandPath deactivatePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType deactivateData;
    deactivateData.analysisStreamID = 0;
    ASSERT_FALSE(mServer.GetLogic().HandleDeactivateAnalysisStream(deactivateHandler, deactivatePath, deactivateData).has_value());
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kWebRTCPendingDeactivation);
    ASSERT_EQ(ImmediateDeactivateStatus(0), Status::InvalidInState);

    // Failure: rejected again
    mFakeWebRTCClient.mLastCallback->OnSessionEnded(Status::Failure, 55);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kFailure);
    ASSERT_EQ(ImmediateDeactivateStatus(0), Status::InvalidInState);
}

TEST_F(TestRemoteAvAnalysisCluster, RemoveIsRejectedOnceTheStreamHasLeftPendingInitiation)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    ActivateStream(0, 2, 55);
    ASSERT_EQ(ImmediateRemoveStatus(0), Status::InvalidInState);
    ASSERT_EQ(mFakeCameraClient.mDeallocationRequests, 0);

    // Failure keeps the camera's stream allocated, so it is not removable either
    mFakeWebRTCClient.mLastCallback->OnSessionFailed(ScopedNodeId(0x1234, 1), 55);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kFailure);
    ASSERT_EQ(ImmediateRemoveStatus(0), Status::InvalidInState);
    ASSERT_EQ(mFakeCameraClient.mDeallocationRequests, 0);
}

TEST_F(TestRemoteAvAnalysisCluster, ActivateOnAnActivatedStreamIsASuccessNoOp)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);

    // Active: SUCCESS with no side-effects
    ASSERT_EQ(ImmediateActivateStatus(0, 2), Status::Success);
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 1);

    // Pending deactivation, transitional: the same, per the spec's "not PendingInitiation or Failure"
    InvalidatableCommandHandler deactivateHandler;
    deactivateHandler.SetFabricIndex(1);
    ConcreteCommandPath deactivatePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType deactivateData;
    deactivateData.analysisStreamID = 0;
    ASSERT_FALSE(mServer.GetLogic().HandleDeactivateAnalysisStream(deactivateHandler, deactivatePath, deactivateData).has_value());
    ASSERT_EQ(ImmediateActivateStatus(0, 2), Status::Success);
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 1);

    // Finish the deactivation so no interaction outlives its handler
    mFakeWebRTCClient.mLastCallback->OnSessionEnded(Status::Success, 55);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kPendingInitiation);
}

TEST_F(TestRemoteAvAnalysisCluster, ActivateWhileAnotherActivateIsParkedIsBusy)
{
    // Two streams of the same camera, distinct camera video streams
    InvalidatableCommandHandler firstEstablish;
    firstEstablish.SetFabricIndex(1);
    EstablishStream(firstEstablish, 0x1234, Status::Success, 42);
    InvalidatableCommandHandler secondEstablish;
    secondEstablish.SetFabricIndex(1);
    EstablishStream(secondEstablish, 0x1234, Status::Success, 43);

    InvalidatableCommandHandler activateHandler;
    activateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
    Commands::ActivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    commandData.webRTCEndpointID = MakeOptional(static_cast<EndpointId>(2));
    ASSERT_FALSE(mServer.GetLogic().HandleActivateAnalysisStream(activateHandler, path, commandData).has_value());

    // One camera-bound command at a time
    ASSERT_EQ(ImmediateActivateStatus(1, 2), Status::Busy);
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 1);

    mFakeWebRTCClient.mLastCallback->OnSessionInitiated(Status::Success, 55, /* aOfferSent = */ true);
    ASSERT_EQ(LastStatus(activateHandler), Status::Success);
}

TEST_F(TestRemoteAvAnalysisCluster, DeactivateWhileAnotherInteractionIsParkedIsBusy)
{
    // An active stream, and a second stream of the same camera whose removal is pending
    InvalidatableCommandHandler firstEstablish;
    firstEstablish.SetFabricIndex(1);
    EstablishStream(firstEstablish, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);
    InvalidatableCommandHandler secondEstablish;
    secondEstablish.SetFabricIndex(1);
    EstablishStream(secondEstablish, 0x1234, Status::Success, 43);

    InvalidatableCommandHandler removeHandler;
    removeHandler.SetFabricIndex(1);
    ConcreteCommandPath removePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType removeData;
    removeData.analysisStreamID = 1;
    ASSERT_FALSE(mServer.GetLogic().HandleRemoveAnalysisStream(removeHandler, removePath, removeData).has_value());

    // One camera-bound command at a time, whichever client carries it
    ASSERT_EQ(ImmediateDeactivateStatus(0), Status::Busy);
    ASSERT_EQ(mFakeWebRTCClient.mEndRequests, 0);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kWebRTCActive);

    mFakeCameraClient.mLastCallback->OnVideoStreamDeallocated(Status::Success, 43);
    ASSERT_EQ(LastStatus(removeHandler), Status::Success);
}

TEST_F(TestRemoteAvAnalysisCluster, SessionFailureDuringDeactivationIsLeftToTheEndSessionOutcome)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);

    InvalidatableCommandHandler deactivateHandler;
    deactivateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    ASSERT_FALSE(mServer.GetLogic().HandleDeactivateAnalysisStream(deactivateHandler, path, commandData).has_value());

    // The camera closes the connection on handling EndSession, before its response arrives: the
    // media layer's failure report is not a transition while the deactivation is pending
    mFakeWebRTCClient.mLastCallback->OnSessionFailed(ScopedNodeId(0x1234, 1), 55);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kWebRTCPendingDeactivation);
    ASSERT_FALSE(deactivateHandler.HasStatus());

    // The EndSession outcome decides
    mFakeWebRTCClient.mLastCallback->OnSessionEnded(Status::Success, 55);
    ASSERT_EQ(LastStatus(deactivateHandler), Status::Success);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kPendingInitiation);
}

TEST_F(TestRemoteAvAnalysisCluster, SessionActiveOnAnAlreadyActiveStreamIsIgnored)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);

    // Only an initiated stream goes active; a repeat report of the same connection changes nothing
    const size_t dirtyBefore = mClusterTester.GetDirtyList().size();
    mFakeWebRTCClient.mLastCallback->OnSessionActive(ScopedNodeId(0x1234, 1), 55);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kWebRTCActive);
    ASSERT_EQ(mClusterTester.GetDirtyList().size(), dirtyBefore);

    // The stream is still active, so it is still deactivatable
    InvalidatableCommandHandler deactivateHandler;
    deactivateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    ASSERT_FALSE(mServer.GetLogic().HandleDeactivateAnalysisStream(deactivateHandler, path, commandData).has_value());
    mFakeWebRTCClient.mLastCallback->OnSessionEnded(Status::Success, 55);
    ASSERT_EQ(LastStatus(deactivateHandler), Status::Success);
}

TEST_F(TestRemoteAvAnalysisCluster, SessionActiveDuringDeactivationIsIgnored)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);

    InvalidatableCommandHandler deactivateHandler;
    deactivateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    ASSERT_FALSE(mServer.GetLogic().HandleDeactivateAnalysisStream(deactivateHandler, path, commandData).has_value());

    // Only an initiated stream goes active: a late connection report leaves the deactivation alone
    const size_t dirtyBefore = mClusterTester.GetDirtyList().size();
    mFakeWebRTCClient.mLastCallback->OnSessionActive(ScopedNodeId(0x1234, 1), 55);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kWebRTCPendingDeactivation);
    ASSERT_EQ(mClusterTester.GetDirtyList().size(), dirtyBefore);
    ASSERT_FALSE(deactivateHandler.HasStatus());

    mFakeWebRTCClient.mLastCallback->OnSessionEnded(Status::Success, 55);
    ASSERT_EQ(LastStatus(deactivateHandler), Status::Success);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kPendingInitiation);
}

TEST_F(TestRemoteAvAnalysisCluster, ExecuteDeactivateAnalysisStreamCommandTest)
{
    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType commandData;

    // An unknown AnalysisStreamID is NOT_FOUND before any state gating
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

    // only an active stream may be deactivated
    InvalidatableCommandHandler establishHandler;
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
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, kCameraNodeId, Status::Success, 42);

    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0; // cluster-assigned id; the camera's id 42 stays internal
    InvalidatableCommandHandler removeHandler;
    removeHandler.SetFabricIndex(1);

    auto response = mServer.GetLogic().HandleRemoveAnalysisStream(removeHandler, path, commandData);
    ASSERT_FALSE(response.has_value()); // Pending on the camera

    // The deallocation goes to the camera the stream was established on
    ASSERT_EQ(mFakeCameraClient.mDeallocationRequests, 1);
    ASSERT_EQ(mFakeCameraClient.mLastCamera, ScopedNodeId(kCameraNodeId, 1));
    ASSERT_EQ(mFakeCameraClient.mLastStreamId, 42);

    mFakeCameraClient.mLastCallback->OnVideoStreamDeallocated(Status::Success, 42);

    ASSERT_TRUE(removeHandler.HasStatus());
    ASSERT_EQ(LastStatus(removeHandler), Status::Success);

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
    InvalidatableCommandHandler commandHandler;
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
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    InvalidatableCommandHandler removeHandler;
    removeHandler.SetFabricIndex(1);

    auto response = mServer.GetLogic().HandleRemoveAnalysisStream(removeHandler, path, commandData);
    ASSERT_FALSE(response.has_value());

    // The camera no longer has the stream, and keeping the entry would occupy capacity that no
    // retry could ever free
    mFakeCameraClient.mLastCallback->OnVideoStreamDeallocated(Status::NotFound, 42);

    ASSERT_EQ(LastStatus(removeHandler), Status::NotFound);

    uint8_t currentCount = 0xFF;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 0);
}

TEST_F(TestRemoteAvAnalysisCluster, RemoveAnalysisStreamPropagatesCameraFailure)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::RemoveAnalysisStream::Id };
    Commands::RemoveAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    InvalidatableCommandHandler removeHandler;
    removeHandler.SetFabricIndex(1);

    auto response = mServer.GetLogic().HandleRemoveAnalysisStream(removeHandler, path, commandData);
    ASSERT_FALSE(response.has_value());

    // The camera's status is returned and the stream entry is kept
    mFakeCameraClient.mLastCallback->OnVideoStreamDeallocated(Status::Failure, 42);

    ASSERT_EQ(LastStatus(removeHandler), Status::Failure);

    uint8_t currentCount = 0;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 1);
}

TEST_F(TestRemoteAvAnalysisCluster, AnalysisStreamsPersistAcrossRestart)
{
    constexpr NodeId kCameraNodeId = 0x1234;
    InvalidatableCommandHandler establishHandler;
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
    InvalidatableCommandHandler removeHandler;
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

TEST_F(TestRemoteAvAnalysisCluster, TrackingEnabledWriteFailsWhenPersistenceFails)
{
    // Poison the KVS key backing TrackingEnabled so its next write fails
    const auto key =
        DefaultStorageKeyAllocator::AttributeValue(kTestEndpointId, Clusters::AvAnalysis::Id, Attributes::TrackingEnabled::Id);
    mClusterTester.GetTestContext().StorageDelegate().AddPoisonKey(key.KeyName());

    const size_t dirtyBefore = mClusterTester.GetDirtyList().size();

    // The value has quality N: a write that cannot be persisted must fail and change nothing
    ASSERT_NE(mServer.GetLogic().SetTrackingEnabled(true), CHIP_NO_ERROR);

    bool trackingEnabled = true;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::TrackingEnabled::Id, trackingEnabled), CHIP_NO_ERROR);
    ASSERT_FALSE(trackingEnabled);
    ASSERT_EQ(mClusterTester.GetDirtyList().size(), dirtyBefore);

    // With storage healthy again the same write goes through
    mClusterTester.GetTestContext().StorageDelegate().ClearPoisonKeys();
    ASSERT_EQ(mServer.GetLogic().SetTrackingEnabled(true), CHIP_NO_ERROR);
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::TrackingEnabled::Id, trackingEnabled), CHIP_NO_ERROR);
    ASSERT_TRUE(trackingEnabled);
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
    InvalidatableCommandHandler secondHandler;
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
    InvalidatableCommandHandler thirdHandler;
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
    InvalidatableCommandHandler removeHandler;
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
    InvalidatableCommandHandler establishHandler;
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

TEST_F(TestRemoteAvAnalysisCluster, DeadExchangeActivateStillRecordsTheSession)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    InvalidatableCommandHandler activateHandler;
    activateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
    Commands::ActivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    commandData.webRTCEndpointID = MakeOptional(static_cast<EndpointId>(2));
    ASSERT_FALSE(mServer.GetLogic().HandleActivateAnalysisStream(activateHandler, path, commandData).has_value());

    // The client exchange dies, then the camera assigns the session. The session is live on the
    // camera regardless and must be recorded, or nothing could ever end it.
    activateHandler.InvalidateHandles();
    mFakeWebRTCClient.mLastCallback->OnSessionInitiated(Status::Success, 55, /* aOfferSent = */ true);
    ASSERT_FALSE(activateHandler.HasStatus());
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kWebRTCInitiated);

    // The recorded session carries the stream through to active and back
    mFakeWebRTCClient.mLastCallback->OnSessionActive(ScopedNodeId(0x1234, 1), 55);
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kWebRTCActive);
    InvalidatableCommandHandler deactivateHandler;
    deactivateHandler.SetFabricIndex(1);
    ConcreteCommandPath deactivatePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType deactivateData;
    deactivateData.analysisStreamID = 0;
    ASSERT_FALSE(mServer.GetLogic().HandleDeactivateAnalysisStream(deactivateHandler, deactivatePath, deactivateData).has_value());
    ASSERT_EQ(mFakeWebRTCClient.mLastSessionId, 55);
    mFakeWebRTCClient.mLastCallback->OnSessionEnded(Status::Success, 55);
    ASSERT_EQ(LastStatus(deactivateHandler), Status::Success);
}

TEST_F(TestRemoteAvAnalysisCluster, DeadExchangeDeactivateStillSettlesTheStream)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);

    InvalidatableCommandHandler deactivateHandler;
    deactivateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    ASSERT_FALSE(mServer.GetLogic().HandleDeactivateAnalysisStream(deactivateHandler, path, commandData).has_value());

    // The client exchange dies, then the camera confirms the end: the stream must not stay
    // PendingDeactivation, a state with no other exit
    deactivateHandler.InvalidateHandles();
    mFakeWebRTCClient.mLastCallback->OnSessionEnded(Status::Success, 55);
    ASSERT_FALSE(deactivateHandler.HasStatus());
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kPendingInitiation);

    ActivateStream(0, 2, 77);
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 2);
}

TEST_F(TestRemoteAvAnalysisCluster, EstablishSameCameraStreamIsIdempotent)
{
    // First stream established with camera-assigned id 42, cluster-assigned AnalysisStreamID 0
    InvalidatableCommandHandler firstHandler;
    firstHandler.SetFabricIndex(1);
    EstablishStream(firstHandler, 0x1234, Status::Success, 42);

    // The camera hands out the same id again, the retry is answered with the existing analysis stream
    // instead of creating a second entry over the same camera stream
    InvalidatableCommandHandler secondHandler;
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
    InvalidatableCommandHandler thirdHandler;
    thirdHandler.SetFabricIndex(1);
    EstablishStream(thirdHandler, 0x5678, Status::Success, 42);
    ASSERT_TRUE(thirdHandler.HasResponse());
    ASSERT_EQ(thirdHandler.DecodeResponse(response), CHIP_NO_ERROR);
    ASSERT_EQ(response.analysisStreamID, 1);
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentAnalysisStreamCount::Id, currentCount), CHIP_NO_ERROR);
    ASSERT_EQ(currentCount, 2);
}

TEST_F(TestRemoteAvAnalysisCluster, StartupRejectsAMisconfiguredInstance)
{
    // A cluster instance is only startable with a coherent feature/resource configuration; each
    // check stops a misconfigured product from booting into an unusable state
    const BitFlags<Feature> kRemote(Feature::kRemoteContextDetection);
    const BitFlags<Feature> kRemoteAndZones(Feature::kRemoteContextDetection, Feature::kPerZoneContextDetection);

    // Neither Local nor Remote detection, and both at once: exactly one is mandatory
    AvAnalysisCluster noDetection(kTestEndpointId, BitFlags<Feature>(), testAmbientContexts, DataModel::NullNullable,
                                  kTestMaxAnalysisStreams);
    noDetection.SetDelegate(&mMockDelegate);
    EXPECT_EQ(noDetection.Startup(mClusterTester.GetServerClusterContext()), CHIP_ERROR_INVALID_ARGUMENT);

    AvAnalysisCluster bothDetection(kTestEndpointId,
                                    BitFlags<Feature>(Feature::kLocalContextDetection, Feature::kRemoteContextDetection),
                                    testAmbientContexts, DataModel::NullNullable, kTestMaxAnalysisStreams);
    bothDetection.SetDelegate(&mMockDelegate);
    EXPECT_EQ(bothDetection.Startup(mClusterTester.GetServerClusterContext()), CHIP_ERROR_INVALID_ARGUMENT);

    // PerZone detection and the zone count go together
    AvAnalysisCluster zonesWithoutMax(kTestEndpointId, kRemoteAndZones, testAmbientContexts, DataModel::NullNullable,
                                      kTestMaxAnalysisStreams);
    zonesWithoutMax.SetDelegate(&mMockDelegate);
    EXPECT_EQ(zonesWithoutMax.Startup(mClusterTester.GetServerClusterContext()), CHIP_ERROR_INVALID_ARGUMENT);

    AvAnalysisCluster maxWithoutZones(kTestEndpointId, kRemote, testAmbientContexts, DataModel::MakeNullable(kTestMaxZones),
                                      kTestMaxAnalysisStreams);
    maxWithoutZones.SetDelegate(&mMockDelegate);
    EXPECT_EQ(maxWithoutZones.Startup(mClusterTester.GetServerClusterContext()), CHIP_ERROR_INVALID_ARGUMENT);

    // Remote detection needs room for at least one analysis stream
    AvAnalysisCluster noStreams(kTestEndpointId, kRemote, testAmbientContexts, DataModel::NullNullable, 0);
    noStreams.SetDelegate(&mMockDelegate);
    EXPECT_EQ(noStreams.Startup(mClusterTester.GetServerClusterContext()), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestRemoteAvAnalysisCluster, StartupRequiresBothClientsWithRemoteDetection)
{
    const BitFlags<Feature> kRemote(Feature::kRemoteContextDetection);

    AvAnalysisCluster noClients(kTestEndpointId, kRemote, testAmbientContexts, DataModel::NullNullable, kTestMaxAnalysisStreams);
    noClients.SetDelegate(&mMockDelegate);
    EXPECT_EQ(noClients.Startup(mClusterTester.GetServerClusterContext()), CHIP_ERROR_INCORRECT_STATE);

    AvAnalysisCluster cameraOnly(kTestEndpointId, kRemote, testAmbientContexts, DataModel::NullNullable, kTestMaxAnalysisStreams);
    cameraOnly.SetDelegate(&mMockDelegate);
    cameraOnly.SetCameraClient(&mFakeCameraClient);
    EXPECT_EQ(cameraOnly.Startup(mClusterTester.GetServerClusterContext()), CHIP_ERROR_INCORRECT_STATE);

    AvAnalysisCluster webRTCOnly(kTestEndpointId, kRemote, testAmbientContexts, DataModel::NullNullable, kTestMaxAnalysisStreams);
    webRTCOnly.SetDelegate(&mMockDelegate);
    webRTCOnly.SetWebRTCClient(&mFakeWebRTCClient);
    EXPECT_EQ(webRTCOnly.Startup(mClusterTester.GetServerClusterContext()), CHIP_ERROR_INCORRECT_STATE);

    AvAnalysisCluster bothClients(kTestEndpointId, kRemote, testAmbientContexts, DataModel::NullNullable, kTestMaxAnalysisStreams);
    bothClients.SetDelegate(&mMockDelegate);
    bothClients.SetCameraClient(&mFakeCameraClient);
    bothClients.SetWebRTCClient(&mFakeWebRTCClient);
    EXPECT_EQ(bothClients.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    bothClients.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestRemoteAvAnalysisCluster, EnableContextTriggersIgnoresRepeatedZones)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 7);

    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;
    uint8_t tlvBuffer[256];

    // A zone named repeatedly is enabled once: stored as given it would also report a list longer
    // than MaxZones, which the persisted size is computed from
    commandData.contextTriggers = EncodeContextTriggers(
        testAmbientContexts.front(), DataModel::MakeNullable(std::vector<uint16_t>{ 2, 1, 2, 1, 2 }), tlvBuffer, sizeof(tlvBuffer));
    ASSERT_TRUE(StatusOf(mServer.GetLogic().HandleEnableContextTriggers(commandHandler, path, commandData)).IsSuccess());

    Attributes::ActiveAmbientContextTriggers::TypeInfo::DecodableType active;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::ActiveAmbientContextTriggers::Id, active), CHIP_NO_ERROR);
    auto iter = active.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_TRUE(iter.GetValue().zoneIDs.HasValue());
    ASSERT_FALSE(iter.GetValue().zoneIDs.Value().IsNull());

    std::vector<uint16_t> storedZones;
    auto zoneIter = iter.GetValue().zoneIDs.Value().Value().begin();
    while (zoneIter.Next())
    {
        storedZones.push_back(zoneIter.GetValue());
    }
    ASSERT_EQ(storedZones, std::vector<uint16_t>({ 1, 2 }));
    ASSERT_FALSE(iter.Next());
}

TEST_F(TestRemoteAvAnalysisCluster, EnableContextTriggersAcceptsAnEmptyZoneList)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 7);

    // No zones to ask about, so the delegate is not consulted: it would refuse if it were
    mMockDelegate.mZoneVerificationResult = CHIP_ERROR_NOT_FOUND;

    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType commandData;
    uint8_t tlvBuffer[128];
    commandData.contextTriggers = EncodeContextTriggers(
        testAmbientContexts.front(), DataModel::MakeNullable(std::vector<uint16_t>{}), tlvBuffer, sizeof(tlvBuffer));

    ASSERT_TRUE(StatusOf(mServer.GetLogic().HandleEnableContextTriggers(commandHandler, path, commandData)).IsSuccess());
}

TEST_F(TestRemoteAvAnalysisCluster, CommandsSurviveALostDelegate)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 7);

    // An instance whose Startup failed stays registered, so a command can arrive with no delegate:
    // every path that would consult it answers a status instead of dereferencing null
    mServer.SetDelegate(nullptr);

    InvalidatableCommandHandler enableHandler;
    enableHandler.SetFabricIndex(1);
    ConcreteCommandPath enablePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType enableData;
    uint8_t tlvBuffer[256];
    enableData.contextTriggers = EncodeContextTriggers(
        testAmbientContexts.front(), DataModel::MakeNullable(std::vector<uint16_t>{ 1 }), tlvBuffer, sizeof(tlvBuffer));
    ASSERT_EQ(
        StatusOf(mServer.GetLogic().HandleEnableContextTriggers(enableHandler, enablePath, enableData)).GetStatusCode().GetStatus(),
        Status::Failure);

    // The null-list branch reaches the same guards without any zones
    InvalidatableCommandHandler nullHandler;
    nullHandler.SetFabricIndex(1);
    Commands::EnableContextTriggers::DecodableType nullData;
    nullData.contextTriggers.SetNull();
    ASSERT_TRUE(StatusOf(mServer.GetLogic().HandleEnableContextTriggers(nullHandler, enablePath, nullData)).IsSuccess());

    InvalidatableCommandHandler disableHandler;
    disableHandler.SetFabricIndex(1);
    ConcreteCommandPath disablePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::DisableContextTriggers::Id };
    Commands::DisableContextTriggers::DecodableType disableData;
    disableData.contextTriggers.SetNull();
    ASSERT_TRUE(StatusOf(mServer.GetLogic().HandleDisableContextTriggers(disableHandler, disablePath, disableData)).IsSuccess());

    // And a session with zones cannot validate them
    uint16_t sessionId = 0;
    ASSERT_EQ(mServer.AnalysisSessionStart(sessionId, DataModel::MakeNullable(std::vector<uint16_t>{ 1 }), 0xCA11, 555),
              CHIP_ERROR_INCORRECT_STATE);

    mServer.SetDelegate(&mMockDelegate);
}

TEST_F(TestRemoteAvAnalysisCluster, AStartupFailureLeavesTheClusterInert)
{
    // Registration only logs a failed Startup, so an instance missing a client stays reachable.
    // Its stream table was never sized and its attributes never loaded: every command answers a
    // status rather than acting or faulting.
    const BitFlags<Feature> kRemote(Feature::kRemoteContextDetection);
    AvAnalysisCluster cameraOnly(kTestEndpointId, kRemote, testAmbientContexts, DataModel::NullNullable, kTestMaxAnalysisStreams);
    cameraOnly.SetDelegate(&mMockDelegate);
    cameraOnly.SetCameraClient(&mFakeCameraClient);
    ASSERT_EQ(cameraOnly.Startup(mClusterTester.GetServerClusterContext()), CHIP_ERROR_INCORRECT_STATE);

    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    ConcreteCommandPath establishPath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EstablishAnalysisStream::Id };
    Commands::EstablishAnalysisStream::DecodableType establishData;
    establishData.nodeID = 0x1234;
    ASSERT_EQ(StatusOf(cameraOnly.GetLogic().HandleEstablishAnalysisStream(establishHandler, establishPath, establishData))
                  .GetStatusCode()
                  .GetStatus(),
              Status::ResourceExhausted);
    ASSERT_EQ(mFakeCameraClient.mAllocationRequests, 0);

    // The gate can never be satisfied, since no stream can be established
    InvalidatableCommandHandler enableHandler;
    enableHandler.SetFabricIndex(1);
    ConcreteCommandPath enablePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::EnableContextTriggers::Id };
    Commands::EnableContextTriggers::DecodableType enableData;
    enableData.contextTriggers.SetNull();
    ASSERT_EQ(StatusOf(cameraOnly.GetLogic().HandleEnableContextTriggers(enableHandler, enablePath, enableData))
                  .GetStatusCode()
                  .GetStatus(),
              Status::InvalidInState);

    // And nothing can be activated, deactivated or removed
    InvalidatableCommandHandler activateHandler;
    activateHandler.SetFabricIndex(1);
    ConcreteCommandPath activatePath{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
    Commands::ActivateAnalysisStream::DecodableType activateData;
    activateData.analysisStreamID = 0;
    activateData.webRTCEndpointID = MakeOptional(static_cast<EndpointId>(1));
    ASSERT_EQ(StatusOf(cameraOnly.GetLogic().HandleActivateAnalysisStream(activateHandler, activatePath, activateData))
                  .GetStatusCode()
                  .GetStatus(),
              Status::NotFound);

    cameraOnly.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestRemoteAvAnalysisCluster, ShutdownWithoutInteractionLeavesTheCameraClientAlone)
{
    // Nothing of ours is in flight on the camera client, so there is nothing for Shutdown to abandon
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
    InvalidatableCommandHandler commandHandler;
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
    InvalidatableCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);

    auto response = mServer.GetLogic().HandleEstablishAnalysisStream(commandHandler, path, commandData);
    ASSERT_FALSE(response.has_value()); // Pending on the camera

    // On a cluster-only shutdown the invoking client is still there; it must be answered rather
    // than left waiting for the interaction to time out
    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);

    ASSERT_TRUE(commandHandler.HasStatus());
    ASSERT_EQ(LastStatus(commandHandler), Status::Failure);

    // Restart so the fixture TearDown shuts down a running server
    EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
}

TEST_F(TestRemoteAvAnalysisCluster, ShutdownCancelsTheWebRTCClientEvenWithNoCommandInFlight)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);

    // The active session's callback is this logic: the client must forget it before the logic goes,
    // while the camera client, with nothing of ours in flight, is left alone
    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);
    ASSERT_EQ(mFakeWebRTCClient.mCancelCount, 1);
    ASSERT_EQ(mFakeCameraClient.mCancelCount, 0);

    // Restart so the fixture TearDown shuts down a running server
    EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
}

TEST_F(TestRemoteAvAnalysisCluster, ShutdownDuringActivateCancelsTheClientsAndAnswersTheCommand)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);

    InvalidatableCommandHandler activateHandler;
    activateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::ActivateAnalysisStream::Id };
    Commands::ActivateAnalysisStream::DecodableType commandData;
    const EndpointId kWebRTCEndpoint = 2;
    commandData.analysisStreamID     = 0;
    commandData.webRTCEndpointID     = MakeOptional(kWebRTCEndpoint);

    auto response = mServer.GetLogic().HandleActivateAnalysisStream(activateHandler, path, commandData);
    ASSERT_FALSE(response.has_value()); // Pending on the offer exchange
    auto * pendingCallback = mFakeWebRTCClient.mLastCallback;
    ASSERT_NE(pendingCallback, nullptr);

    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);
    ASSERT_GE(mFakeWebRTCClient.mCancelCount, 1);
    ASSERT_GE(mFakeCameraClient.mCancelCount, 1);
    ASSERT_EQ(LastStatus(activateHandler), Status::Failure);

    // A stray late completion (contract violation by a client) answers nothing more
    pendingCallback->OnSessionInitiated(Status::Success, 55, /* aOfferSent = */ true);
    ASSERT_EQ(activateHandler.GetStatuses().size(), 1u);

    // Restart so the fixture TearDown shuts down a running server
    EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
}

TEST_F(TestRemoteAvAnalysisCluster, ShutdownDuringDeactivateAnswersTheCommandAndIgnoresTheLateEnd)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);

    InvalidatableCommandHandler deactivateHandler;
    deactivateHandler.SetFabricIndex(1);
    ConcreteCommandPath path{ kTestEndpointId, Clusters::AvAnalysis::Id, Commands::DeactivateAnalysisStream::Id };
    Commands::DeactivateAnalysisStream::DecodableType commandData;
    commandData.analysisStreamID = 0;
    ASSERT_FALSE(mServer.GetLogic().HandleDeactivateAnalysisStream(deactivateHandler, path, commandData).has_value());
    auto * pendingCallback = mFakeWebRTCClient.mLastCallback;
    ASSERT_NE(pendingCallback, nullptr);

    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);
    ASSERT_GE(mFakeWebRTCClient.mCancelCount, 1);
    ASSERT_EQ(LastStatus(deactivateHandler), Status::Failure);

    // A stray late completion answers nothing more
    pendingCallback->OnSessionEnded(Status::Success, 55);
    ASSERT_EQ(deactivateHandler.GetStatuses().size(), 1u);

    // The WebRTC client forgot the session, so the stream is settled with it, before any restart
    ASSERT_EQ(FirstStreamState(), AnalysisStreamStateEnum::kPendingInitiation);

    // Restart so the fixture TearDown shuts down a running server
    EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
}

TEST_F(TestRemoteAvAnalysisCluster, ShutdownSettlesActiveStreamsToPendingInitiation)
{
    InvalidatableCommandHandler establishHandler;
    establishHandler.SetFabricIndex(1);
    EstablishStream(establishHandler, 0x1234, Status::Success, 42);
    ActivateStream(0, 2, 55);

    // The client forgets the active session on Shutdown: the stream no longer refers to it
    mServer.Shutdown(ClusterShutdownType::kClusterShutdown);
    ASSERT_EQ(mFakeWebRTCClient.mCancelCount, 1);

    Attributes::AnalysisStreams::TypeInfo::DecodableType streams;
    ASSERT_EQ(mClusterTester.ReadAttribute(Attributes::AnalysisStreams::Id, streams), CHIP_NO_ERROR);
    auto iter = streams.begin();
    ASSERT_TRUE(iter.Next());
    ASSERT_EQ(iter.GetValue().analysisStreamState, AnalysisStreamStateEnum::kPendingInitiation);
    ASSERT_TRUE(iter.GetValue().webRTCEndpointID.HasValue());
    ASSERT_TRUE(iter.GetValue().webRTCEndpointID.Value().IsNull());

    // Restart: the stream is activatable again
    EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    ActivateStream(0, 2, 77);
    ASSERT_EQ(mFakeWebRTCClient.mSessionRequests, 2);
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

TEST_F(TestRemoteAvAnalysisCluster, AnalysisStreamTableDecodeRejectsOutOfRangeEntryId)
{
    AnalysisStreamTable table;
    ASSERT_EQ(table.Init(2), CHIP_NO_ERROR);
    const ScopedNodeId kCamera(0xABCD, 1);
    ASSERT_NE(table.Add(10, kCamera), nullptr);

    // Forge an entry id the generator can never mint (the ceiling is 65534)
    table[0].analysisStreamID = 65535;

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
    // leaves the counter above 65534, so accepting
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
