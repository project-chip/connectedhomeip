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
#include <clusters/Descriptor/Attributes.h>
#include <clusters/Descriptor/Ids.h>
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

    void SetUp() override { ASSERT_EQ(mClient.Init(&mCASESessionManager), CHIP_NO_ERROR); }

    // Never used for real sessions: InterceptingCameraClient overrides EstablishSession.
    CASESessionManager mCASESessionManager;
    InterceptingCameraClient mClient;
    RecordingCallback mCallback;
};

TEST_F(TestDefaultAvAnalysisCameraClient, InitArgumentValidation)
{
    DefaultAvAnalysisCameraClient client;
    EXPECT_EQ(client.Init(nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);
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
    mClient.OnDone(static_cast<CommandSender *>(nullptr));

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
    mClient.OnDone(static_cast<CommandSender *>(nullptr));

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CameraErrorStatusIsPropagated)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);

    // Spec 11.9.8: the camera's status code (e.g. RESOURCE_EXHAUSTED) is propagated verbatim
    mClient.OnError(nullptr, StatusIB(Status::ResourceExhausted).ToChipError());
    mClient.OnDone(static_cast<CommandSender *>(nullptr));

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::ResourceExhausted);
}

TEST_F(TestDefaultAvAnalysisCameraClient, DeallocationSuccessReportsStreamId)
{
    EXPECT_EQ(mClient.RequestVideoStreamDeallocation(kCameraNode, 9, mCallback), CHIP_NO_ERROR);

    ConcreteCommandPath responsePath(kCameraEndpoint, CameraAvStreamManagement::Id,
                                     CameraAvStreamManagement::Commands::VideoStreamDeallocate::Id);
    mClient.OnResponse(nullptr, responsePath, StatusIB(), nullptr);
    mClient.OnDone(static_cast<CommandSender *>(nullptr));

    EXPECT_EQ(mCallback.mDeallocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
    EXPECT_EQ(mCallback.mLastStreamId, 9);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CompletionIsDeliveredExactlyOnce)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);

    mClient.OnError(nullptr, StatusIB(Status::Failure).ToChipError());
    mClient.OnDone(static_cast<CommandSender *>(nullptr)); // OnDone after OnError must not produce a second callback

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
}

// Exposes the protected request builder and profile for direct testing
class ProfileTestClient : public DefaultAvAnalysisCameraClient
{
public:
    using DefaultAvAnalysisCameraClient::AnalysisUsageSupported;
    using DefaultAvAnalysisCameraClient::BuildAllocateRequest;
    using DefaultAvAnalysisCameraClient::CameraProfile;
    using DefaultAvAnalysisCameraClient::CurrentProfile;
    using DefaultAvAnalysisCameraClient::FillProfileDefaults;
    using DefaultAvAnalysisCameraClient::HandleCapabilityReport;
    using DefaultAvAnalysisCameraClient::HandleServerListReport;
    using DefaultAvAnalysisCameraClient::OnProfileDiscoveryComplete;

protected:
    // Discovery does not run on its own; the test drives OnProfileDiscoveryComplete explicitly
    void StartProfileDiscovery() override {}
    void EstablishSession(const ScopedNodeId & aCameraNode) override {}
};

TEST_F(TestDefaultAvAnalysisCameraClient, AllocateRequestFollowsProfileBounds)
{
    ProfileTestClient::CameraProfile profile;
    profile.avsmEndpoint  = 5;
    profile.hasWatermark  = true;
    profile.hasOSD        = false;
    profile.minFrameRate  = 10;
    profile.maxFrameRate  = 24;
    profile.minWidth      = 320;
    profile.minHeight     = 240;
    profile.maxWidth      = 1280;
    profile.maxHeight     = 720;
    profile.minBitRateBps = 100000;
    profile.maxBitRateBps = 1500000;

    auto request = ProfileTestClient::BuildAllocateRequest(profile);

    EXPECT_EQ(request.streamUsage, Globals::StreamUsageEnum::kAnalysis);
    EXPECT_EQ(request.minFrameRate, 10);
    EXPECT_EQ(request.maxFrameRate, 24);
    EXPECT_EQ(request.minResolution.width, 320);
    EXPECT_EQ(request.maxResolution.width, 1280);
    EXPECT_EQ(request.maxResolution.height, 720);
    EXPECT_EQ(request.minBitRate, 100000u);
    EXPECT_EQ(request.maxBitRate, 1500000u);

    ASSERT_TRUE(request.watermarkEnabled.HasValue());
    EXPECT_FALSE(request.watermarkEnabled.Value());
    EXPECT_FALSE(request.OSDEnabled.HasValue());
}

TEST_F(TestDefaultAvAnalysisCameraClient, DefaultProfileHasSaneBoundsAndNoCameraSpecifics)
{
    ProfileTestClient::CameraProfile profile;
    ProfileTestClient::FillProfileDefaults(profile);

    // Camera-specific values come only from discovery
    EXPECT_EQ(profile.avsmEndpoint, kInvalidEndpointId);
    EXPECT_FALSE(profile.hasWatermark);
    EXPECT_FALSE(profile.hasOSD);
    EXPECT_GT(profile.maxFrameRate, profile.minFrameRate);
    EXPECT_GT(profile.maxBitRateBps, profile.minBitRateBps);
    EXPECT_GE(profile.maxWidth, profile.minWidth);
}

TEST_F(TestDefaultAvAnalysisCameraClient, DiscoveryFailureFailsRequest)
{
    ProfileTestClient client;
    ASSERT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);

    EXPECT_EQ(client.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);
    EXPECT_EQ(mCallback.mAllocatedCount, 0); // Nothing completes until discovery does

    client.OnProfileDiscoveryComplete(CHIP_ERROR_TIMEOUT);

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);

    // Client is reusable after the failed discovery
    EXPECT_EQ(client.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisCameraClient, DiscoverySuccessWithoutSessionFailsRequest)
{
    ProfileTestClient client;
    ASSERT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);

    EXPECT_EQ(client.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);

    // Discovery reporting success without a held session cannot send the command
    client.OnProfileDiscoveryComplete(CHIP_NO_ERROR);

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CancelDropsPendingRequestWithoutCompletion)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);

    mClient.Cancel();

    // Late interaction events after Cancel must not produce a completion
    mClient.OnError(nullptr, StatusIB(Status::Failure).ToChipError());
    mClient.OnDone(static_cast<CommandSender *>(nullptr));
    EXPECT_EQ(mCallback.mAllocatedCount, 0);

    // The client accepts a new request after cancellation
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);
    EXPECT_EQ(mClient.mConnectRequests, 2);
}

// Encodes a Descriptor ServerList value (array of cluster ids) into aBuffer and positions aReader on it
void EncodeServerList(const ClusterId * aClusters, size_t aCount, uint8_t * aBuffer, size_t aBufferSize, TLV::TLVReader & aReader)
{
    TLV::TLVWriter writer;
    writer.Init(aBuffer, aBufferSize);
    TLV::TLVType containerType;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, containerType), CHIP_NO_ERROR);
    for (size_t i = 0; i < aCount; i++)
    {
        ASSERT_EQ(writer.Put(TLV::AnonymousTag(), aClusters[i]), CHIP_NO_ERROR);
    }
    ASSERT_EQ(writer.EndContainer(containerType), CHIP_NO_ERROR);

    aReader.Init(aBuffer, writer.GetLengthWritten());
    ASSERT_EQ(aReader.Next(), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisCameraClient, DiscoveryFindsAvsmEndpointInServerList)
{
    ProfileTestClient client;
    ASSERT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);

    uint8_t buffer[64];
    TLV::TLVReader reader;

    // Endpoint 0 serves Descriptor but not CameraAVStreamManagement: not a match
    const ClusterId kRootClusters[] = { Descriptor::Id };
    EncodeServerList(kRootClusters, MATTER_ARRAY_SIZE(kRootClusters), buffer, sizeof(buffer), reader);
    ConcreteDataAttributePath rootPath(0, Descriptor::Id, Descriptor::Attributes::ServerList::Id);
    client.HandleServerListReport(rootPath, reader);
    EXPECT_EQ(client.CurrentProfile().avsmEndpoint, kInvalidEndpointId);

    // Endpoint 3 serves CameraAVStreamManagement: discovered
    const ClusterId kCameraClusters[] = { Descriptor::Id, CameraAvStreamManagement::Id };
    EncodeServerList(kCameraClusters, MATTER_ARRAY_SIZE(kCameraClusters), buffer, sizeof(buffer), reader);
    ConcreteDataAttributePath cameraPath(3, Descriptor::Id, Descriptor::Attributes::ServerList::Id);
    client.HandleServerListReport(cameraPath, reader);
    EXPECT_EQ(client.CurrentProfile().avsmEndpoint, 3);

    // First match wins: a later endpoint with the cluster does not overwrite it
    EncodeServerList(kCameraClusters, MATTER_ARRAY_SIZE(kCameraClusters), buffer, sizeof(buffer), reader);
    ConcreteDataAttributePath laterPath(4, Descriptor::Id, Descriptor::Attributes::ServerList::Id);
    client.HandleServerListReport(laterPath, reader);
    EXPECT_EQ(client.CurrentProfile().avsmEndpoint, 3);
}

// Positions aReader on an anonymous TLV element encoded by aEncode
template <typename EncodeFn>
void EncodeTlv(uint8_t * aBuffer, size_t aBufferSize, TLV::TLVReader & aReader, EncodeFn aEncode)
{
    TLV::TLVWriter writer;
    writer.Init(aBuffer, aBufferSize);
    ASSERT_EQ(aEncode(writer), CHIP_NO_ERROR);
    aReader.Init(aBuffer, writer.GetLengthWritten());
    ASSERT_EQ(aReader.Next(), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CapabilityReadDerivesTraitsFromFeatureMap)
{
    ProfileTestClient client;
    ASSERT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);

    uint8_t buffer[64];
    TLV::TLVReader reader;
    const uint32_t featureMap = to_underlying(CameraAvStreamManagement::Feature::kWatermark);
    EncodeTlv(buffer, sizeof(buffer), reader, [&](TLV::TLVWriter & w) { return w.Put(TLV::AnonymousTag(), featureMap); });

    // The profile's endpoint is unset (kInvalidEndpointId), so the report
    // path must use the same endpoint to be accepted
    ConcreteDataAttributePath path(kInvalidEndpointId, CameraAvStreamManagement::Id, Globals::Attributes::FeatureMap::Id);
    client.HandleCapabilityReport(path, reader);

    EXPECT_TRUE(client.CurrentProfile().hasWatermark);
    EXPECT_FALSE(client.CurrentProfile().hasOSD);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CapabilityReadBoundsResolutionAndFrameRate)
{
    ProfileTestClient client;
    ASSERT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);

    uint8_t buffer[64];
    TLV::TLVReader reader;
    CameraAvStreamManagement::Structs::VideoSensorParamsStruct::Type sensorParams;
    sensorParams.sensorWidth  = 1280;
    sensorParams.sensorHeight = 720;
    sensorParams.maxFPS       = 25;
    EncodeTlv(buffer, sizeof(buffer), reader,
              [&](TLV::TLVWriter & w) { return DataModel::Encode(w, TLV::AnonymousTag(), sensorParams); });

    ConcreteDataAttributePath path(kInvalidEndpointId, CameraAvStreamManagement::Id,
                                   CameraAvStreamManagement::Attributes::VideoSensorParams::Id);
    client.HandleCapabilityReport(path, reader);

    EXPECT_EQ(client.CurrentProfile().maxWidth, 1280);
    EXPECT_EQ(client.CurrentProfile().maxHeight, 720);
    EXPECT_EQ(client.CurrentProfile().maxFrameRate, 25);
    // Min bounds never exceed the discovered max
    EXPECT_LE(client.CurrentProfile().minWidth, client.CurrentProfile().maxWidth);
    EXPECT_LE(client.CurrentProfile().minFrameRate, client.CurrentProfile().maxFrameRate);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CapabilityReadTakesMinimumsFromTradeOffPoints)
{
    ProfileTestClient client;
    ASSERT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);

    CameraAvStreamManagement::Structs::RateDistortionTradeOffPointsStruct::Type points[2];
    points[0].codec             = CameraAvStreamManagement::VideoCodecEnum::kH264;
    points[0].resolution.width  = 640;
    points[0].resolution.height = 360;
    points[0].minBitRate        = 250000;
    points[1].codec             = CameraAvStreamManagement::VideoCodecEnum::kHevc;
    points[1].resolution.width  = 320;
    points[1].resolution.height = 180;
    points[1].minBitRate        = 100000;

    uint8_t buffer[128];
    TLV::TLVReader reader;
    EncodeTlv(buffer, sizeof(buffer), reader, [&](TLV::TLVWriter & w) {
        TLV::TLVType outer;
        ReturnErrorOnFailure(w.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outer));
        for (const auto & point : points)
        {
            ReturnErrorOnFailure(DataModel::Encode(w, TLV::AnonymousTag(), point));
        }
        return w.EndContainer(outer);
    });

    ConcreteDataAttributePath path(kInvalidEndpointId, CameraAvStreamManagement::Id,
                                   CameraAvStreamManagement::Attributes::RateDistortionTradeOffPoints::Id);
    client.HandleCapabilityReport(path, reader);

    // Only the H.264 point counts; the HEVC one must be ignored
    EXPECT_EQ(client.CurrentProfile().minBitRateBps, 250000u);
    EXPECT_EQ(client.CurrentProfile().minWidth, 640);
    EXPECT_EQ(client.CurrentProfile().minHeight, 360);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CapabilityReadDetectsMissingAnalysisUsage)
{
    ProfileTestClient client;
    ASSERT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);
    EXPECT_TRUE(client.AnalysisUsageSupported());

    uint8_t buffer[32];
    TLV::TLVReader reader;
    EncodeTlv(buffer, sizeof(buffer), reader, [](TLV::TLVWriter & w) {
        TLV::TLVType outer;
        ReturnErrorOnFailure(w.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outer));
        ReturnErrorOnFailure(w.Put(TLV::AnonymousTag(), Globals::StreamUsageEnum::kLiveView));
        ReturnErrorOnFailure(w.Put(TLV::AnonymousTag(), Globals::StreamUsageEnum::kRecording));
        return w.EndContainer(outer);
    });

    ConcreteDataAttributePath path(kInvalidEndpointId, CameraAvStreamManagement::Id,
                                   CameraAvStreamManagement::Attributes::SupportedStreamUsages::Id);
    client.HandleCapabilityReport(path, reader);

    EXPECT_FALSE(client.AnalysisUsageSupported());
}

TEST_F(TestDefaultAvAnalysisCameraClient, DoneWithoutResponseIsFailure)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);

    mClient.OnDone(static_cast<CommandSender *>(nullptr)); // Interaction ended with neither response nor error

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
}

} // namespace
