/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/av-analysis-server/DefaultAvAnalysisCameraClient.h>
#include <clusters/CameraAvStreamManagement/Commands.h>
#include <clusters/CameraAvStreamManagement/Ids.h>
#include <lib/core/TLV.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace Protocols::InteractionModel;

constexpr EndpointId kCameraEndpoint = 1;
const ScopedNodeId kCameraNode(0x1234, 1);

// Intercepts the network boundary: records session requests instead of establishing CASE sessions.
// The camera's behavior is simulated by invoking the public CommandSender::Callback methods.
class InterceptingCameraClient : public DefaultAvAnalysisCameraClient
{
public:
    int mConnectRequests = 0;
    ScopedNodeId mLastPeer;

protected:
    void EstablishSession(const ScopedNodeId & aCameraNode) override
    {
        mConnectRequests++;
        mLastPeer = aCameraNode;
    }
};

class RecordingCallback : public AvAnalysisCameraClient::Callback
{
public:
    int mAllocatedCount    = 0;
    int mDeallocatedCount  = 0;
    Status mLastStatus     = Status::Success;
    uint16_t mLastStreamId = 0;

    void OnVideoStreamAllocated(Status aStatus, uint16_t aVideoStreamId) override
    {
        mAllocatedCount++;
        mLastStatus   = aStatus;
        mLastStreamId = aVideoStreamId;
    }

    void OnVideoStreamDeallocated(Status aStatus, uint16_t aAnalysisStreamId) override
    {
        mDeallocatedCount++;
        mLastStatus   = aStatus;
        mLastStreamId = aAnalysisStreamId;
    }
};

struct TestDefaultAvAnalysisCameraClient : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(mClient.Init(&mCASESessionManager, kCameraEndpoint), CHIP_NO_ERROR); }

    // Never used for real sessions: InterceptingCameraClient overrides EstablishSession.
    CASESessionManager mCASESessionManager;
    InterceptingCameraClient mClient;
    RecordingCallback mCallback;
};

TEST_F(TestDefaultAvAnalysisCameraClient, InitArgumentValidation)
{
    DefaultAvAnalysisCameraClient client;
    EXPECT_EQ(client.Init(nullptr, kCameraEndpoint), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(client.Init(&mCASESessionManager, kInvalidEndpointId), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(client.Init(&mCASESessionManager, kCameraEndpoint), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisCameraClient, RequestStartsSessionEstablishment)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);
    EXPECT_EQ(mClient.mConnectRequests, 1);
    EXPECT_EQ(mClient.mLastPeer, kCameraNode);
    EXPECT_EQ(mCallback.mAllocatedCount, 0); // Nothing completed yet
}

TEST_F(TestDefaultAvAnalysisCameraClient, SecondRequestWhilePendingIsBusy)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);
    EXPECT_EQ(mClient.RequestVideoStreamDeallocation(kCameraNode, 5, mCallback), CHIP_ERROR_BUSY);
    EXPECT_EQ(mClient.mConnectRequests, 1);
}

TEST_F(TestDefaultAvAnalysisCameraClient, AllocationSuccessDecodesVideoStreamId)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);

    // Craft the camera's VideoStreamAllocateResponse{VideoStreamID=7} payload
    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType containerType;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType), CHIP_NO_ERROR);
    ASSERT_EQ(writer.Put(TLV::ContextTag(0), static_cast<uint16_t>(7)), CHIP_NO_ERROR);
    ASSERT_EQ(writer.EndContainer(containerType), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);

    ConcreteCommandPath responsePath(kCameraEndpoint, CameraAvStreamManagement::Id,
                                     CameraAvStreamManagement::Commands::VideoStreamAllocateResponse::Id);
    mClient.OnResponse(nullptr, responsePath, StatusIB(), &reader);
    mClient.OnDone(nullptr);

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
    EXPECT_EQ(mCallback.mLastStreamId, 7);

    // Completed: the client accepts a new request again
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisCameraClient, AllocationSuccessWithoutPayloadIsFailure)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);

    ConcreteCommandPath responsePath(kCameraEndpoint, CameraAvStreamManagement::Id,
                                     CameraAvStreamManagement::Commands::VideoStreamAllocateResponse::Id);
    mClient.OnResponse(nullptr, responsePath, StatusIB(), nullptr);
    mClient.OnDone(nullptr);

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CameraErrorStatusIsPropagated)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);

    // Spec 11.9.8: the camera's status code (e.g. RESOURCE_EXHAUSTED) is propagated verbatim
    mClient.OnError(nullptr, StatusIB(Status::ResourceExhausted).ToChipError());
    mClient.OnDone(nullptr);

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::ResourceExhausted);
}

TEST_F(TestDefaultAvAnalysisCameraClient, DeallocationSuccessReportsStreamId)
{
    EXPECT_EQ(mClient.RequestVideoStreamDeallocation(kCameraNode, 9, mCallback), CHIP_NO_ERROR);

    ConcreteCommandPath responsePath(kCameraEndpoint, CameraAvStreamManagement::Id,
                                     CameraAvStreamManagement::Commands::VideoStreamDeallocate::Id);
    mClient.OnResponse(nullptr, responsePath, StatusIB(), nullptr);
    mClient.OnDone(nullptr);

    EXPECT_EQ(mCallback.mDeallocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
    EXPECT_EQ(mCallback.mLastStreamId, 9);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CompletionIsDeliveredExactlyOnce)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);

    mClient.OnError(nullptr, StatusIB(Status::Failure).ToChipError());
    mClient.OnDone(nullptr); // OnDone after OnError must not produce a second callback

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CancelDropsPendingRequestWithoutCompletion)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);

    mClient.Cancel();

    // Late interaction events after Cancel must not produce a completion
    mClient.OnError(nullptr, StatusIB(Status::Failure).ToChipError());
    mClient.OnDone(nullptr);
    EXPECT_EQ(mCallback.mAllocatedCount, 0);

    // The client accepts a new request after cancellation
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);
    EXPECT_EQ(mClient.mConnectRequests, 2);
}

TEST_F(TestDefaultAvAnalysisCameraClient, DoneWithoutResponseIsFailure)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);

    mClient.OnDone(nullptr); // Interaction ended with neither response nor error

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
}

} // namespace
