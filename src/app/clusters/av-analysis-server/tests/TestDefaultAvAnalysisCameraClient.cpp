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

CommandSender * const kTestSender = reinterpret_cast<CommandSender *>(0xC0FFEE);

// Intercepts the network boundary: records session requests instead of establishing CASE sessions.
// The camera's behavior is simulated by invoking the public CommandSender::Callback methods.
class InterceptingCameraClient : public DefaultAvAnalysisCameraClient
{
public:
    int mConnectRequests = 0;
    ScopedNodeId mLastPeer;

    // Pretends the request reached the camera through aSender. The pointer is only ever compared,
    // never dereferenced, so tests can hand in a distinct non-null value per interaction.
    void SimulateCommandSent(CommandSender * aSender)
    {
        CurrentRequest().SetInvokedSender(aSender);
        CurrentRequest().Advance(Request::Phase::kInvoking);
    }

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

    void OnVideoStreamDeallocated(Status aStatus, uint16_t aVideoStreamId) override
    {
        mDeallocatedCount++;
        mLastStatus   = aStatus;
        mLastStreamId = aVideoStreamId;
    }
};

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

// Encodes a Descriptor ServerList value and positions aReader on it
void EncodeServerList(Span<const ClusterId> aClusters, MutableByteSpan aBuffer, TLV::TLVReader & aReader)
{
    EncodeTlv(aBuffer.data(), aBuffer.size(), aReader, [aClusters](TLV::TLVWriter & w) {
        TLV::TLVType outer;
        ReturnErrorOnFailure(w.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outer));
        for (ClusterId cluster : aClusters)
        {
            ReturnErrorOnFailure(w.Put(TLV::AnonymousTag(), cluster));
        }
        return w.EndContainer(outer);
    });
}

// Exposes the protected request builder and profile for direct testing
class ProfileTestClient : public DefaultAvAnalysisCameraClient
{
public:
    using DefaultAvAnalysisCameraClient::BuildAllocateRequest;
    using DefaultAvAnalysisCameraClient::CameraProfile;
    using DefaultAvAnalysisCameraClient::CurrentProfile;
    using DefaultAvAnalysisCameraClient::CurrentRequest;
    using DefaultAvAnalysisCameraClient::HandleCapabilityReport;
    using DefaultAvAnalysisCameraClient::HandleServerListReport;
    using DefaultAvAnalysisCameraClient::NormalizeProfile;
    using DefaultAvAnalysisCameraClient::OnProfileDiscoveryComplete;

protected:
    // Discovery does not run on its own; the test drives OnProfileDiscoveryComplete explicitly
    void StartProfileDiscovery() override {}
    void EstablishSession(const ScopedNodeId & aCameraNode) override {}
};

// Encodes a TLV array of values, the shape every list-valued capability attribute uses
template <typename T>
CHIP_ERROR EncodeArray(TLV::TLVWriter & aWriter, Span<const T> aValues)
{
    TLV::TLVType outer;
    ReturnErrorOnFailure(aWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outer));
    for (const T & value : aValues)
    {
        ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::AnonymousTag(), value));
    }
    return aWriter.EndContainer(outer);
}

// Feeds one AVSM capability report into the client, as the capabilities read would.
template <typename EncodeFn>
void FeedCapability(ProfileTestClient & aClient, AttributeId aAttributeId, EncodeFn aEncode)
{
    uint8_t buffer[192];
    TLV::TLVReader reader;
    EncodeTlv(buffer, sizeof(buffer), reader, aEncode);
    aClient.HandleCapabilityReport(ConcreteDataAttributePath(kInvalidEndpointId, CameraAvStreamManagement::Id, aAttributeId),
                                   reader);
}

class ReentrantCallback : public AvAnalysisCameraClient::Callback
{
public:
    InterceptingCameraClient * mClient = nullptr;
    bool mStartAnother                 = true;
    int mAllocatedCount                = 0;
    int mDeallocatedCount              = 0;
    Status mLastStatus                 = Status::Success;

    void OnVideoStreamAllocated(Status aStatus, uint16_t aVideoStreamId) override
    {
        mAllocatedCount++;
        mLastStatus = aStatus;
        if (mStartAnother)
        {
            mStartAnother = false;
            EXPECT_EQ(mClient->RequestVideoStreamDeallocation(kCameraNode, 9, *this), CHIP_NO_ERROR);
        }
    }

    void OnVideoStreamDeallocated(Status aStatus, uint16_t aVideoStreamId) override
    {
        mDeallocatedCount++;
        mLastStatus = aStatus;
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
    mClient.SimulateCommandSent(kTestSender);

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
    mClient.OnResponse(kTestSender, responsePath, StatusIB(), &reader);
    mClient.OnDone(kTestSender);

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
    EXPECT_EQ(mCallback.mLastStreamId, 7);

    // Completed: the client accepts a new request again
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisCameraClient, AllocationSuccessWithoutPayloadIsFailure)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);
    mClient.SimulateCommandSent(kTestSender);

    ConcreteCommandPath responsePath(kCameraEndpoint, CameraAvStreamManagement::Id,
                                     CameraAvStreamManagement::Commands::VideoStreamAllocateResponse::Id);
    mClient.OnResponse(kTestSender, responsePath, StatusIB(), nullptr);
    mClient.OnDone(kTestSender);

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CameraErrorStatusIsPropagated)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);
    mClient.SimulateCommandSent(kTestSender);

    // Spec 11.9.8: the camera's status code (e.g. RESOURCE_EXHAUSTED) is propagated verbatim
    mClient.OnError(kTestSender, StatusIB(Status::ResourceExhausted).ToChipError());
    mClient.OnDone(kTestSender);

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::ResourceExhausted);
}

TEST_F(TestDefaultAvAnalysisCameraClient, DeallocationSuccessReportsStreamId)
{
    EXPECT_EQ(mClient.RequestVideoStreamDeallocation(kCameraNode, 9, mCallback), CHIP_NO_ERROR);
    mClient.SimulateCommandSent(kTestSender);

    ConcreteCommandPath responsePath(kCameraEndpoint, CameraAvStreamManagement::Id,
                                     CameraAvStreamManagement::Commands::VideoStreamDeallocate::Id);
    mClient.OnResponse(kTestSender, responsePath, StatusIB(), nullptr);
    mClient.OnDone(kTestSender);

    EXPECT_EQ(mCallback.mDeallocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
    EXPECT_EQ(mCallback.mLastStreamId, 9);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CompletionIsDeliveredExactlyOnce)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);
    mClient.SimulateCommandSent(kTestSender);

    mClient.OnError(kTestSender, StatusIB(Status::Failure).ToChipError());
    mClient.OnDone(kTestSender); // OnDone after OnError must not produce a second callback

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
}

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
    mClient.SimulateCommandSent(kTestSender);

    mClient.Cancel();

    // Late interaction events after Cancel must not produce a completion
    mClient.OnError(kTestSender, StatusIB(Status::Failure).ToChipError());
    mClient.OnDone(kTestSender);
    EXPECT_EQ(mCallback.mAllocatedCount, 0);

    // The client accepts a new request after cancellation
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);
    EXPECT_EQ(mClient.mConnectRequests, 2);
}

// Feeds the two bound-carrying capability reports in the given order and returns the reconciled
// profile, so a test can prove the outcome does not depend on which arrived first.
DefaultAvAnalysisCameraClient::CameraProfile ReconcileBounds(bool aSensorFirst)
{
    ProfileTestClient client;
    CASESessionManager caseSessionManager;
    VerifyOrDie(client.Init(&caseSessionManager) == CHIP_NO_ERROR);

    const auto feedSensor = [&client] {
        CameraAvStreamManagement::Structs::VideoSensorParamsStruct::Type sensorParams;
        sensorParams.sensorWidth  = 1920;
        sensorParams.sensorHeight = 1080;
        sensorParams.maxFPS       = 30;
        FeedCapability(client, CameraAvStreamManagement::Attributes::VideoSensorParams::Id,
                       [&](TLV::TLVWriter & w) { return DataModel::Encode(w, TLV::AnonymousTag(), sensorParams); });
    };
    // A self-inconsistent camera: its smallest encodable point exceeds its own sensor
    const auto feedTradeOffPoint = [&client] {
        using TradeOffPoint = CameraAvStreamManagement::Structs::RateDistortionTradeOffPointsStruct::Type;
        TradeOffPoint point;
        point.codec             = CameraAvStreamManagement::VideoCodecEnum::kH264;
        point.resolution.width  = 3840;
        point.resolution.height = 2160;
        point.minBitRate        = 8000000;
        FeedCapability(client, CameraAvStreamManagement::Attributes::RateDistortionTradeOffPoints::Id,
                       [&](TLV::TLVWriter & w) { return EncodeArray(w, Span<const TradeOffPoint>(&point, 1)); });
    };

    if (aSensorFirst)
    {
        feedSensor();
        feedTradeOffPoint();
    }
    else
    {
        feedTradeOffPoint();
        feedSensor();
    }

    DefaultAvAnalysisCameraClient::CameraProfile profile = client.CurrentProfile();
    ProfileTestClient::NormalizeProfile(profile);
    return profile;
}

TEST_F(TestDefaultAvAnalysisCameraClient, ReconciledBoundsDoNotDependOnReportOrder)
{
    const auto sensorFirst = ReconcileBounds(/* aSensorFirst = */ true);
    const auto pointsFirst = ReconcileBounds(/* aSensorFirst = */ false);

    // Attribute reports have no guaranteed order, so the same data must reconcile the same way
    EXPECT_EQ(sensorFirst.minWidth, pointsFirst.minWidth);
    EXPECT_EQ(sensorFirst.maxWidth, pointsFirst.maxWidth);
    EXPECT_EQ(sensorFirst.minHeight, pointsFirst.minHeight);
    EXPECT_EQ(sensorFirst.maxHeight, pointsFirst.maxHeight);
    EXPECT_EQ(sensorFirst.minBitRateBps, pointsFirst.minBitRateBps);
    EXPECT_EQ(sensorFirst.maxBitRateBps, pointsFirst.maxBitRateBps);

    // The sensor is the camera's ceiling, so the resolution never exceeds what it reported
    EXPECT_EQ(sensorFirst.maxWidth, 1920);
    EXPECT_EQ(sensorFirst.minWidth, 1920);
    // Nothing publishes a maximum bit rate, so the default rises to what the camera needs
    EXPECT_EQ(sensorFirst.minBitRateBps, 8000000u);
    EXPECT_EQ(sensorFirst.maxBitRateBps, 8000000u);
}

TEST_F(TestDefaultAvAnalysisCameraClient, NormalizeProfileKeepsBoundsSatisfiable)
{
    // Camera-reported minimums can land above the maximums; such a range cannot be requested.
    using Profile = ProfileTestClient::CameraProfile;
    Profile profile;
    profile.minWidth      = 3840;
    profile.minHeight     = 2160;
    profile.minFrameRate  = 60;
    profile.minBitRateBps = 8000000;

    ProfileTestClient::NormalizeProfile(profile);

    EXPECT_EQ(profile.minWidth, Profile::kDefaultMaxWidth);
    EXPECT_EQ(profile.maxWidth, Profile::kDefaultMaxWidth);
    EXPECT_EQ(profile.minHeight, Profile::kDefaultMaxHeight);
    EXPECT_EQ(profile.minFrameRate, Profile::kDefaultMaxFrameRate);

    EXPECT_EQ(profile.minBitRateBps, 8000000u);
    EXPECT_EQ(profile.maxBitRateBps, 8000000u);

    auto request = ProfileTestClient::BuildAllocateRequest(profile);
    EXPECT_GE(request.maxResolution.width, request.minResolution.width);
    EXPECT_GE(request.maxResolution.height, request.minResolution.height);
    EXPECT_GE(request.maxBitRate, request.minBitRate);
    EXPECT_GE(request.maxFrameRate, request.minFrameRate);
}

TEST_F(TestDefaultAvAnalysisCameraClient, DiscoveryFindsAvsmEndpointInServerList)
{
    ProfileTestClient client;
    ASSERT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);

    uint8_t buffer[64];
    TLV::TLVReader reader;

    // Endpoint 0 serves Descriptor but not CameraAVStreamManagement: not a match
    const ClusterId kRootClusters[] = { Descriptor::Id };
    EncodeServerList(Span<const ClusterId>(kRootClusters), MutableByteSpan(buffer), reader);
    ConcreteDataAttributePath rootPath(0, Descriptor::Id, Descriptor::Attributes::ServerList::Id);
    client.HandleServerListReport(rootPath, reader);
    EXPECT_EQ(client.CurrentProfile().avsmEndpoint, kInvalidEndpointId);

    // Endpoint 3 serves CameraAVStreamManagement: discovered
    const ClusterId kCameraClusters[] = { Descriptor::Id, CameraAvStreamManagement::Id };
    EncodeServerList(Span<const ClusterId>(kCameraClusters), MutableByteSpan(buffer), reader);
    ConcreteDataAttributePath cameraPath(3, Descriptor::Id, Descriptor::Attributes::ServerList::Id);
    client.HandleServerListReport(cameraPath, reader);
    EXPECT_EQ(client.CurrentProfile().avsmEndpoint, 3);

    // First match wins: a later endpoint with the cluster does not overwrite it
    EncodeServerList(Span<const ClusterId>(kCameraClusters), MutableByteSpan(buffer), reader);
    ConcreteDataAttributePath laterPath(4, Descriptor::Id, Descriptor::Attributes::ServerList::Id);
    client.HandleServerListReport(laterPath, reader);
    EXPECT_EQ(client.CurrentProfile().avsmEndpoint, 3);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CapabilityReadDerivesTraitsFromFeatureMap)
{
    ProfileTestClient client;
    ASSERT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);

    const uint32_t featureMap = to_underlying(CameraAvStreamManagement::Feature::kWatermark);
    FeedCapability(client, Globals::Attributes::FeatureMap::Id,
                   [&](TLV::TLVWriter & w) { return w.Put(TLV::AnonymousTag(), featureMap); });

    EXPECT_TRUE(client.CurrentProfile().hasWatermark);
    EXPECT_FALSE(client.CurrentProfile().hasOSD);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CapabilityReadBoundsResolutionAndFrameRate)
{
    ProfileTestClient client;
    ASSERT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);

    CameraAvStreamManagement::Structs::VideoSensorParamsStruct::Type sensorParams;
    sensorParams.sensorWidth  = 320;
    sensorParams.sensorHeight = 240;
    sensorParams.maxFPS       = 10;
    FeedCapability(client, CameraAvStreamManagement::Attributes::VideoSensorParams::Id,
                   [&](TLV::TLVWriter & w) { return DataModel::Encode(w, TLV::AnonymousTag(), sensorParams); });

    EXPECT_EQ(client.CurrentProfile().maxWidth, 320);
    EXPECT_EQ(client.CurrentProfile().maxHeight, 240);
    EXPECT_EQ(client.CurrentProfile().maxFrameRate, 10);

    // Reconciliation happens once, where the request is built
    DefaultAvAnalysisCameraClient::CameraProfile profile = client.CurrentProfile();
    ProfileTestClient::NormalizeProfile(profile);

    // The defaults (640x480 at 15fps) exceeded the sensor, so they were clamped down to it
    EXPECT_EQ(profile.minWidth, 320);
    EXPECT_EQ(profile.minHeight, 240);
    EXPECT_EQ(profile.minFrameRate, 10);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CapabilityReadTakesMinimumsFromTradeOffPoints)
{
    ProfileTestClient client;
    ASSERT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);

    using TradeOffPoint = CameraAvStreamManagement::Structs::RateDistortionTradeOffPointsStruct::Type;
    TradeOffPoint points[2];
    // Values deliberately unlike the profile defaults, so each assertion below can fail
    points[0].codec             = CameraAvStreamManagement::VideoCodecEnum::kH264;
    points[0].resolution.width  = 512;
    points[0].resolution.height = 288;
    points[0].minBitRate        = 250000;
    points[1].codec             = CameraAvStreamManagement::VideoCodecEnum::kHevc;
    points[1].resolution.width  = 320;
    points[1].resolution.height = 180;
    points[1].minBitRate        = 100000;

    FeedCapability(client, CameraAvStreamManagement::Attributes::RateDistortionTradeOffPoints::Id,
                   [&](TLV::TLVWriter & w) { return EncodeArray(w, Span<const TradeOffPoint>(points)); });

    // Only the H.264 point counts; the HEVC one must be ignored
    EXPECT_EQ(client.CurrentProfile().minBitRateBps, 250000u);
    EXPECT_EQ(client.CurrentProfile().minWidth, 512);
    EXPECT_EQ(client.CurrentProfile().minHeight, 288);
}

TEST_F(TestDefaultAvAnalysisCameraClient, EveryCapabilityAttributeIsRequiredForAnAllocation)
{
    ProfileTestClient client;
    ASSERT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);
    EXPECT_FALSE(client.CurrentRequest().AllCapabilitiesReported());

    const uint32_t featureMap = to_underlying(CameraAvStreamManagement::Feature::kWatermark);
    FeedCapability(client, Globals::Attributes::FeatureMap::Id,
                   [&](TLV::TLVWriter & w) { return w.Put(TLV::AnonymousTag(), featureMap); });
    EXPECT_FALSE(client.CurrentRequest().AllCapabilitiesReported());

    CameraAvStreamManagement::Structs::VideoSensorParamsStruct::Type sensorParams;
    sensorParams.sensorWidth  = 1280;
    sensorParams.sensorHeight = 720;
    sensorParams.maxFPS       = 25;
    FeedCapability(client, CameraAvStreamManagement::Attributes::VideoSensorParams::Id,
                   [&](TLV::TLVWriter & w) { return DataModel::Encode(w, TLV::AnonymousTag(), sensorParams); });
    EXPECT_FALSE(client.CurrentRequest().AllCapabilitiesReported());

    using TradeOffPoint = CameraAvStreamManagement::Structs::RateDistortionTradeOffPointsStruct::Type;
    TradeOffPoint point;
    point.codec             = CameraAvStreamManagement::VideoCodecEnum::kH264;
    point.resolution.width  = 640;
    point.resolution.height = 360;
    point.minBitRate        = 250000;
    FeedCapability(client, CameraAvStreamManagement::Attributes::RateDistortionTradeOffPoints::Id,
                   [&](TLV::TLVWriter & w) { return EncodeArray(w, Span<const TradeOffPoint>(&point, 1)); });
    EXPECT_FALSE(client.CurrentRequest().AllCapabilitiesReported());

    const Globals::StreamUsageEnum usages[] = { Globals::StreamUsageEnum::kAnalysis };
    FeedCapability(client, CameraAvStreamManagement::Attributes::SupportedStreamUsages::Id,
                   [&](TLV::TLVWriter & w) { return EncodeArray(w, Span<const Globals::StreamUsageEnum>(usages)); });

    EXPECT_TRUE(client.CurrentRequest().AllCapabilitiesReported());
    EXPECT_EQ(client.CurrentRequest().MissingCapabilities(), 0);
}

TEST_F(TestDefaultAvAnalysisCameraClient, CapabilityReadDetectsMissingAnalysisUsage)
{
    ProfileTestClient client;
    ASSERT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);
    EXPECT_TRUE(client.CurrentRequest().AnalysisUsageSupported());

    const Globals::StreamUsageEnum usages[] = { Globals::StreamUsageEnum::kLiveView, Globals::StreamUsageEnum::kRecording };
    FeedCapability(client, CameraAvStreamManagement::Attributes::SupportedStreamUsages::Id,
                   [&](TLV::TLVWriter & w) { return EncodeArray(w, Span<const Globals::StreamUsageEnum>(usages)); });

    EXPECT_FALSE(client.CurrentRequest().AnalysisUsageSupported());
}

TEST_F(TestDefaultAvAnalysisCameraClient, DoneWithoutResponseIsFailure)
{
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, mCallback), CHIP_NO_ERROR);
    mClient.SimulateCommandSent(kTestSender);

    mClient.OnDone(kTestSender); // Interaction ended with neither response nor error

    EXPECT_EQ(mCallback.mAllocatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
}

TEST_F(TestDefaultAvAnalysisCameraClient, StaleInteractionDoesNotFailTheNextRequest)
{
    // A CommandSender always reports OnDone after OnResponse. If the response's completion starts a
    // new request, that trailing OnDone belongs to the finished interaction and must not touch it.
    auto * firstSender  = reinterpret_cast<CommandSender *>(0x1);
    auto * secondSender = reinterpret_cast<CommandSender *>(0x2);

    ReentrantCallback callback;
    callback.mClient = &mClient;

    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, callback), CHIP_NO_ERROR);
    mClient.SimulateCommandSent(firstSender);

    ConcreteCommandPath responsePath(kCameraEndpoint, CameraAvStreamManagement::Id,
                                     CameraAvStreamManagement::Commands::VideoStreamAllocateResponse::Id);
    mClient.OnResponse(firstSender, responsePath, StatusIB(), nullptr);
    ASSERT_EQ(callback.mAllocatedCount, 1);

    // The deallocation started from that completion is now in flight
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, callback), CHIP_ERROR_BUSY);

    // The first interaction's trailing OnDone must leave it alone
    mClient.OnDone(firstSender);
    EXPECT_EQ(callback.mDeallocatedCount, 0);
    EXPECT_EQ(mClient.RequestVideoStreamAllocation(kCameraNode, callback), CHIP_ERROR_BUSY);

    // The second interaction still completes normally
    mClient.SimulateCommandSent(secondSender);
    mClient.OnError(secondSender, StatusIB(Status::NotFound).ToChipError());
    EXPECT_EQ(callback.mDeallocatedCount, 1);
    EXPECT_EQ(callback.mLastStatus, Status::NotFound);
}

TEST_F(TestDefaultAvAnalysisCameraClient, AttributeReportWithoutARequestIsIgnored)
{
    ProfileTestClient client;
    ASSERT_EQ(client.Init(&mCASESessionManager), CHIP_NO_ERROR);

    uint8_t buffer[64];
    TLV::TLVReader reader;
    const ClusterId kCameraClusters[] = { Descriptor::Id, CameraAvStreamManagement::Id };
    EncodeServerList(Span<const ClusterId>(kCameraClusters), MutableByteSpan(buffer), reader);

    // No request is in flight, so there is no discovery phase to route this report to
    ConcreteDataAttributePath path(3, Descriptor::Id, Descriptor::Attributes::ServerList::Id);
    client.OnAttributeData(path, &reader, StatusIB());

    EXPECT_EQ(client.CurrentProfile().avsmEndpoint, kInvalidEndpointId);
}

} // namespace
