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

#include <app/AttributeValueDecoder.h>
#include <app/CommandHandler.h>
#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/camera-av-stream-management-server/CameraAVStreamManagementCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model/Decode.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/CameraAvStreamManagement/Attributes.h>
#include <clusters/CameraAvStreamManagement/Commands.h>
#include <clusters/CameraAvStreamManagement/Enums.h>
#include <clusters/CameraAvStreamManagement/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <protocols/interaction_model/StatusCode.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvStreamManagement;
using namespace chip::Testing;

static constexpr chip::EndpointId kTestEndpointId = 1;

static AudioCapabilitiesStruct & GetAudioCapabilities()
{
    static std::array<AudioCodecEnum, 2> audioCodecs = { AudioCodecEnum::kOpus, AudioCodecEnum::kAacLc };
    static std::array<uint32_t, 2> sampleRates       = { 48000, 32000 }; // Sample rates in Hz
    static std::array<uint8_t, 2> bitDepths          = { 24, 32 };
    static AudioCapabilitiesStruct audioCapabilities = { 2, chip::Span<AudioCodecEnum>(audioCodecs),
                                                         chip::Span<uint32_t>(sampleRates), chip::Span<uint8_t>(bitDepths) };
    return audioCapabilities;
}
static VideoSensorParamsStruct & GetVideoSensorParams()
{
    static VideoSensorParamsStruct videoSensorParams = { 1920, 1080, 120,
                                                         chip::Optional<uint16_t>(30) }; // Typical numbers for Pi camera.
    return videoSensorParams;
}

static std::vector<RateDistortionTradeOffStruct> & GetRateDistortionTradeOffPoints()
{
    static std::vector<RateDistortionTradeOffStruct> rateDistTradeOffs = {
        { VideoCodecEnum::kH264, { 640, 480 }, 10000 /* bitrate */ }
    };
    return rateDistTradeOffs;
}

static std::vector<SnapshotCapabilitiesStruct> & GetSnapshotCapabilities()
{
    static std::vector<SnapshotCapabilitiesStruct> snapshotCapabilities = {
        { { 640, 480 }, 30, ImageCodecEnum::kJpeg, false, chip::MakeOptional(false) },
        { { 1280, 720 }, 30, ImageCodecEnum::kJpeg, true, chip::MakeOptional(true) },
    };
    return snapshotCapabilities;
}

static std::vector<StreamUsageEnum> & GetSupportedStreamUsages()
{
    static std::vector<StreamUsageEnum> supportedStreamUsage = { StreamUsageEnum::kLiveView, StreamUsageEnum::kRecording };
    return supportedStreamUsage;
}

// Mock delegate for testing CameraAVStreamManagement
class MockCameraAVStreamManagementDelegate : public CameraAVStreamManagementDelegate
{
public:
    MockCameraAVStreamManagementDelegate(std::vector<VideoStreamStruct> * videoStreams,
                                         std::vector<AudioStreamStruct> * audioStreams,
                                         std::vector<SnapshotStreamStruct> * snapshotStreams) :
        mAllocatedVideoStreams(videoStreams),
        mAllocatedAudioStreams(audioStreams), mAllocatedSnapshotStreams(snapshotStreams), mAudioStreamCount(0),
        mVideoStreamCount(0), mSnapshotStreamCount(0)
    {}

    Protocols::InteractionModel::Status VideoStreamAllocate(const VideoStreamStruct & allocateArgs, uint16_t & outStreamID) override
    {
        if (!std::any_of(GetRateDistortionTradeOffPoints().begin(), GetRateDistortionTradeOffPoints().end(),
                         [&](const auto & tradeOffPoint) { return tradeOffPoint.codec == allocateArgs.videoCodec; }))
        {
            return Protocols::InteractionModel::Status::DynamicConstraintError;
        }

        VerifyOrReturnError(mVideoStreamCount < 1, Protocols::InteractionModel::Status::ResourceExhausted);

        outStreamID = static_cast<uint16_t>(mAllocatedVideoStreams->size() + 1);
        mAllocatedVideoStreams->push_back(allocateArgs);
        mVideoStreamCount++;
        return Protocols::InteractionModel::Status::Success;
    }

    void OnVideoStreamAllocated(const VideoStreamStruct & allocatedStream, StreamAllocationAction action) override {}

    Protocols::InteractionModel::Status VideoStreamModify(const uint16_t streamID, const Optional<bool> waterMarkEnabled,
                                                          const Optional<bool> osdEnabled) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status VideoStreamDeallocate(const uint16_t streamID) override
    {
        mVideoStreamCount--;
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status AudioStreamAllocate(const AudioStreamStruct & allocateArgs, uint16_t & outStreamID) override
    {
        VerifyOrReturnError(mAudioStreamCount < 1, Protocols::InteractionModel::Status::ResourceExhausted);

        auto & audioCapabilities = GetAudioCapabilities();

        if (!std::any_of(audioCapabilities.supportedCodecs.begin(), audioCapabilities.supportedCodecs.end(),
                         [&](const auto & codec) { return codec == allocateArgs.audioCodec; }))
        {
            return Protocols::InteractionModel::Status::DynamicConstraintError;
        }

        if (!std::any_of(audioCapabilities.supportedSampleRates.begin(), audioCapabilities.supportedSampleRates.end(),
                         [&](const auto & sampleRate) { return sampleRate == allocateArgs.sampleRate; }))
        {
            return Protocols::InteractionModel::Status::DynamicConstraintError;
        }

        if (!std::any_of(audioCapabilities.supportedBitDepths.begin(), audioCapabilities.supportedBitDepths.end(),
                         [&](const auto & bitDepth) { return bitDepth == allocateArgs.bitDepth; }))
        {
            return Protocols::InteractionModel::Status::DynamicConstraintError;
        }

        outStreamID = static_cast<uint16_t>(mAllocatedAudioStreams->size() + 1);
        mAllocatedAudioStreams->push_back(allocateArgs);
        mAudioStreamCount++;
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status AudioStreamDeallocate(const uint16_t streamID) override
    {
        mAudioStreamCount--;
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status SnapshotStreamAllocate(const SnapshotStreamAllocateArgs & allocateArgs,
                                                               uint16_t & outStreamID) override
    {
        VerifyOrReturnError(mSnapshotStreamCount < 1, Protocols::InteractionModel::Status::ResourceExhausted);

        if (!std::any_of(GetSnapshotCapabilities().begin(), GetSnapshotCapabilities().end(),
                         [&](const auto & capability) { return capability.imageCodec == allocateArgs.imageCodec; }))
        {
            return Protocols::InteractionModel::Status::DynamicConstraintError;
        }

        outStreamID = static_cast<uint16_t>(mAllocatedSnapshotStreams->size() + 1);

        SnapshotStreamStruct newStream;
        newStream.snapshotStreamID = outStreamID;
        newStream.imageCodec       = allocateArgs.imageCodec;
        newStream.frameRate        = allocateArgs.maxFrameRate;
        newStream.minResolution    = allocateArgs.minResolution;
        newStream.maxResolution    = allocateArgs.maxResolution;
        newStream.quality          = allocateArgs.quality;
        newStream.encodedPixels    = allocateArgs.encodedPixels;
        newStream.hardwareEncoder  = allocateArgs.hardwareEncoder;
        newStream.watermarkEnabled = allocateArgs.watermarkEnabled;
        newStream.OSDEnabled       = allocateArgs.OSDEnabled;
        newStream.referenceCount   = 1;

        mAllocatedSnapshotStreams->push_back(newStream);
        mSnapshotStreamCount++;
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status SnapshotStreamModify(const uint16_t streamID, const Optional<bool> waterMarkEnabled,
                                                             const Optional<bool> osdEnabled) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status SnapshotStreamDeallocate(const uint16_t streamID) override
    {
        mSnapshotStreamCount--;
        return Protocols::InteractionModel::Status::Success;
    }

    void OnStreamUsagePrioritiesChanged() override {}

    void OnAttributeChanged(AttributeId attributeId) override {}

    Protocols::InteractionModel::Status CaptureSnapshot(const DataModel::Nullable<uint16_t> streamID,
                                                        const VideoResolutionStruct & resolution,
                                                        ImageSnapshot & outImageSnapshot) override
    {
        outImageSnapshot.imageCodec    = ImageCodecEnum::kJpeg;
        outImageSnapshot.imageRes      = resolution;
        const uint8_t dummyImageData[] = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01, 0x01, 0x00, 0x00,
                                           0x01, 0x00, 0x01, 0x00, 0x00, 0xFF, 0xDB, 0x00, 0x43, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x00, 0x11, 0x08,
                                           0x00, 0x01, 0x00, 0x01, 0x03, 0x01, 0x22, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01, 0xFF,
                                           0xC4, 0x00, 0x1F, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                                           0x0A, 0x0B, 0xFF, 0xDA, 0x00, 0x0C, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3F,
                                           0x00, 0xF2, 0x8A, 0x28, 0xFF, 0xD9 };
        outImageSnapshot.data.assign(dummyImageData, dummyImageData + sizeof(dummyImageData));
        return Protocols::InteractionModel::Status::Success;
    }

    CHIP_ERROR PersistentAttributesLoadedCallback() override { return CHIP_NO_ERROR; }

    const std::vector<VideoStreamStruct> & GetAllocatedVideoStreams() const override { return *mAllocatedVideoStreams; }

    const std::vector<AudioStreamStruct> & GetAllocatedAudioStreams() const override { return *mAllocatedAudioStreams; }

    const std::vector<SnapshotStreamStruct> & GetAllocatedSnapshotStreams() const { return *mAllocatedSnapshotStreams; }

private:
    std::vector<VideoStreamStruct> * mAllocatedVideoStreams;
    std::vector<AudioStreamStruct> * mAllocatedAudioStreams;
    std::vector<SnapshotStreamStruct> * mAllocatedSnapshotStreams;
    uint8_t mAudioStreamCount;
    uint8_t mVideoStreamCount;
    uint8_t mSnapshotStreamCount;
};

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestCameraAVStreamManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    static CHIP_ERROR InitializeCameraAVSMDefaults(CameraAvStreamManagement::CameraAVStreamManagementCluster & mServer)
    {
        ReturnErrorOnFailure(mServer.SetSoftRecordingPrivacyModeEnabled(true));
        ReturnErrorOnFailure(mServer.SetSoftLivestreamPrivacyModeEnabled(true));
        ReturnErrorOnFailure(mServer.SetHardPrivacyModeOn(true));
        ReturnErrorOnFailure(mServer.SetNightVision(TriStateAutoEnum::kAuto));
        ReturnErrorOnFailure(mServer.SetViewport({ 0, 0, 1920, 1080 }));
        ReturnErrorOnFailure(mServer.SetSpeakerMuted(true));
        ReturnErrorOnFailure(mServer.SetSpeakerVolumeLevel(1));
        ReturnErrorOnFailure(mServer.SetSpeakerMinLevel(1));
        ReturnErrorOnFailure(mServer.SetSpeakerMaxLevel(254));
        ReturnErrorOnFailure(mServer.SetMicrophoneMuted(true));
        ReturnErrorOnFailure(mServer.SetMicrophoneVolumeLevel(1));
        ReturnErrorOnFailure(mServer.SetMicrophoneMinLevel(1));
        ReturnErrorOnFailure(mServer.SetMicrophoneMaxLevel(254));
        ReturnErrorOnFailure(mServer.SetMicrophoneAGCEnabled(true));
        ReturnErrorOnFailure(mServer.SetImageRotation(0));
        ReturnErrorOnFailure(mServer.SetImageFlipHorizontal(false));
        ReturnErrorOnFailure(mServer.SetImageFlipVertical(false));
        ReturnErrorOnFailure(mServer.SetStatusLightEnabled(true));
        ReturnErrorOnFailure(mServer.SetStatusLightBrightness(Globals::ThreeLevelAutoEnum::kMedium));

        return CHIP_NO_ERROR;
    }

    TestCameraAVStreamManagementCluster() :
        mMockDelegate(&mVideoStreams, &mAudioStreams, &mSnapshotStreams),
        mServer(CameraAvStreamManagement::CameraAVStreamManagementCluster::Context{ mPersistenceProvider }, mMockDelegate,
                kTestEndpointId,
                chip::BitFlags<CameraAvStreamManagement::Feature>(
                    CameraAvStreamManagement::Feature::kVideo, CameraAvStreamManagement::Feature::kAudio,
                    CameraAvStreamManagement::Feature::kSnapshot, CameraAvStreamManagement::Feature::kSpeaker,
                    CameraAvStreamManagement::Feature::kImageControl, CameraAvStreamManagement::Feature::kPrivacy,
                    CameraAvStreamManagement::Feature::kWatermark, CameraAvStreamManagement::Feature::kHighDynamicRange,
                    CameraAvStreamManagement::Feature::kNightVision),
                chip::BitFlags<CameraAvStreamManagement::OptionalAttribute>(
                    CameraAvStreamManagement::OptionalAttribute::kHardPrivacyModeOn,
                    CameraAvStreamManagement::OptionalAttribute::kMicrophoneAGCEnabled,
                    CameraAvStreamManagement::OptionalAttribute::kImageRotation,
                    CameraAvStreamManagement::OptionalAttribute::kImageFlipHorizontal,
                    CameraAvStreamManagement::OptionalAttribute::kImageFlipVertical,
                    CameraAvStreamManagement::OptionalAttribute::kStatusLightEnabled,
                    CameraAvStreamManagement::OptionalAttribute::kStatusLightBrightness),
                1, 248832000 /*1920*1080*120 */, GetVideoSensorParams(), false, { 640, 480 }, GetRateDistortionTradeOffPoints(),
                4096, GetAudioCapabilities(), GetAudioCapabilities(), TwoWayTalkSupportTypeEnum::kFullDuplex,
                GetSnapshotCapabilities(), 128000000, GetSupportedStreamUsages(), GetSupportedStreamUsages()),
        mClusterTester(mServer)
    {}

    void SetUp() override
    {
        VerifyOrDie(mPersistenceProvider.Init(&mClusterTester.GetServerClusterContext().storage) == CHIP_NO_ERROR);
        EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
        EXPECT_EQ(InitializeCameraAVSMDefaults(mServer), CHIP_NO_ERROR);
        EXPECT_EQ(mServer.Init(), CHIP_NO_ERROR);
    }

    std::vector<VideoStreamStruct> mVideoStreams;
    std::vector<AudioStreamStruct> mAudioStreams;
    std::vector<SnapshotStreamStruct> mSnapshotStreams;
    MockCameraAVStreamManagementDelegate mMockDelegate;
    app::DefaultSafeAttributePersistenceProvider mPersistenceProvider;
    CameraAvStreamManagement::CameraAVStreamManagementCluster mServer;
    ClusterTester mClusterTester;
};

TEST_F(TestCameraAVStreamManagementCluster, TestAttributes)
{
    ASSERT_TRUE(
        Testing::IsAttributesListEqualTo(mServer,
                                         {
                                             CameraAvStreamManagement::Attributes::MaxConcurrentEncoders::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MaxEncodedPixelRate::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::VideoSensorParams::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::NightVisionUsesInfrared::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MinViewportResolution::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::RateDistortionTradeOffPoints::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MaxContentBufferSize::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MicrophoneCapabilities::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::SpeakerCapabilities::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::TwoWayTalkSupport::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::SnapshotCapabilities::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MaxNetworkBandwidth::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::CurrentFrameRate::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::HDRModeEnabled::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::SupportedStreamUsages::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::AllocatedVideoStreams::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::AllocatedAudioStreams::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::AllocatedSnapshotStreams::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::StreamUsagePriorities::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::SoftRecordingPrivacyModeEnabled::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::SoftLivestreamPrivacyModeEnabled::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::HardPrivacyModeOn::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::NightVision::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::Viewport::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::SpeakerMuted::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::SpeakerVolumeLevel::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::SpeakerMaxLevel::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::SpeakerMinLevel::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MicrophoneMuted::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MicrophoneVolumeLevel::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MicrophoneMaxLevel::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MicrophoneMinLevel::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MicrophoneAGCEnabled::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::ImageRotation::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::ImageFlipHorizontal::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::ImageFlipVertical::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::StatusLightEnabled::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::StatusLightBrightness::kMetadataEntry,
                                         }));
}

TEST_F(TestCameraAVStreamManagementCluster, TestAcceptedCommands)
{
    ASSERT_TRUE(
        Testing::IsAcceptedCommandsListEqualTo(mServer,
                                               {
                                                   CameraAvStreamManagement::Commands::VideoStreamAllocate::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::VideoStreamDeallocate::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::VideoStreamModify::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::AudioStreamAllocate::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::AudioStreamDeallocate::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::SnapshotStreamAllocate::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::SnapshotStreamDeallocate::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::SnapshotStreamModify::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::SetStreamPriorities::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::CaptureSnapshot::kMetadataEntry,

                                               }));
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadClusterRevisionAttribute)
{
    // Create a mock attribute request for ClusterRevision
    chip::app::DataModel::ReadAttributeRequest request;
    request.path.mEndpointId  = kTestEndpointId;
    request.path.mClusterId   = CameraAvStreamManagement::Id;
    request.path.mAttributeId = chip::app::Clusters::Globals::Attributes::ClusterRevision::Id;

    // Create a buffer for encoding
    chip::Platform::ScopedMemoryBufferWithSize<uint8_t> buffer;
    ASSERT_TRUE(buffer.Alloc(1024));

    // Create AttributeReportIBs::Builder for the encoder
    chip::app::AttributeReportIBs::Builder attributeReportIBsBuilder;
    chip::TLV::TLVWriter reportWriter;
    reportWriter.Init(buffer.Get(), buffer.AllocatedSize());
    CHIP_ERROR err = attributeReportIBsBuilder.Init(&reportWriter);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    chip::app::AttributeValueEncoder encoder(attributeReportIBsBuilder, chip::Access::SubjectDescriptor{}, request.path,
                                             0 /* dataVersion */);

    // Test reading cluster revision
    auto status = mServer.ReadAttribute(request, encoder);
    EXPECT_TRUE(status.IsSuccess());
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadMaxConcurrentEncoders)
{
    uint8_t maxConcurrentEncoders = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MaxConcurrentEncoders::Id, maxConcurrentEncoders), CHIP_NO_ERROR);
    EXPECT_EQ(maxConcurrentEncoders, 1);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadMaxEncodedPixelRate)
{
    uint32_t maxEncodedPixelRate = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MaxEncodedPixelRate::Id, maxEncodedPixelRate), CHIP_NO_ERROR);
    EXPECT_EQ(maxEncodedPixelRate, static_cast<uint32_t>(248832000));
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadVideoSensorParams)
{
    Attributes::VideoSensorParams::TypeInfo::DecodableType videoSensorParams;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::VideoSensorParams::Id, videoSensorParams), CHIP_NO_ERROR);
    EXPECT_EQ(videoSensorParams.sensorWidth, 1920);
    EXPECT_EQ(videoSensorParams.sensorHeight, 1080);
    EXPECT_EQ(videoSensorParams.maxFPS, 120);
    EXPECT_TRUE(videoSensorParams.maxHDRFPS.HasValue());
    EXPECT_EQ(videoSensorParams.maxHDRFPS.Value(), 30);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadNightVisionUsesInfrared)
{
    bool usesInfrared = true; // Default Init is false
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::NightVisionUsesInfrared::Id, usesInfrared), CHIP_NO_ERROR);
    EXPECT_FALSE(usesInfrared);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadMinViewportResolution)
{
    Attributes::MinViewportResolution::TypeInfo::Type minViewportResolution;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MinViewportResolution::Id, minViewportResolution), CHIP_NO_ERROR);
    EXPECT_EQ(minViewportResolution.width, 640);
    EXPECT_EQ(minViewportResolution.height, 480);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadRateDistortionTradeOffPoints)
{
    Attributes::RateDistortionTradeOffPoints::TypeInfo::DecodableType rateDistortionTradeOffPoints;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::RateDistortionTradeOffPoints::Id, rateDistortionTradeOffPoints),
              CHIP_NO_ERROR);
    auto it       = rateDistortionTradeOffPoints.begin();
    uint8_t count = 0;
    while (it.Next())
    {
        ++count;
    }
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(count, 1u);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadMaxContentBufferSize)
{
    uint32_t maxContentBufferSize = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MaxContentBufferSize::Id, maxContentBufferSize), CHIP_NO_ERROR);
    EXPECT_EQ(maxContentBufferSize, 4096u);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadMicrophoneCapabilities)
{
    Attributes::MicrophoneCapabilities::TypeInfo::DecodableType microphoneCapabilities;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MicrophoneCapabilities::Id, microphoneCapabilities), CHIP_NO_ERROR);

    EXPECT_EQ(microphoneCapabilities.maxNumberOfChannels, 2);

    auto it       = microphoneCapabilities.supportedCodecs.begin();
    uint8_t count = 0;
    while (it.Next())
    {
        ++count;
    }
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(count, 2u);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadSpeakerCapabilities)
{
    Attributes::SpeakerCapabilities::TypeInfo::DecodableType speakerCapabilities;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SpeakerCapabilities::Id, speakerCapabilities), CHIP_NO_ERROR);

    EXPECT_EQ(speakerCapabilities.maxNumberOfChannels, 2);

    auto it       = speakerCapabilities.supportedCodecs.begin();
    uint8_t count = 0;
    while (it.Next())
    {
        ++count;
    }
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(count, 2u);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadTwoWayTalkSupport)
{
    Attributes::TwoWayTalkSupport::TypeInfo::Type twoWayTalkSupport = TwoWayTalkSupportTypeEnum::kNotSupported;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::TwoWayTalkSupport::Id, twoWayTalkSupport), CHIP_NO_ERROR);
    EXPECT_EQ(twoWayTalkSupport, TwoWayTalkSupportTypeEnum::kFullDuplex);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadSnapshotCapabilities)
{
    Attributes::SnapshotCapabilities::TypeInfo::DecodableType snapshotCapabilities;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SnapshotCapabilities::Id, snapshotCapabilities), CHIP_NO_ERROR);
    auto it       = snapshotCapabilities.begin();
    uint8_t count = 0;
    while (it.Next())
    {
        ++count;
    }
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(count, 2u);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadMaxNetworkBandwidth)
{
    uint32_t maxNetworkBandwidth = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MaxNetworkBandwidth::Id, maxNetworkBandwidth), CHIP_NO_ERROR);
    EXPECT_EQ(maxNetworkBandwidth, 128000000u);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadCurrentFrameRate)
{
    Attributes::CurrentFrameRate::TypeInfo::Type currentFrameRate = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::CurrentFrameRate::Id, currentFrameRate), CHIP_NO_ERROR);
    EXPECT_EQ(currentFrameRate, 0);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadWriteHDRModeEnabled)
{
    bool hdrModeEnabled = false;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::HDRModeEnabled::Id, hdrModeEnabled), CHIP_NO_ERROR);
    EXPECT_FALSE(hdrModeEnabled); // Default should be false

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::HDRModeEnabled::Id, true), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::HDRModeEnabled::Id, hdrModeEnabled), CHIP_NO_ERROR);
    EXPECT_TRUE(hdrModeEnabled);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::HDRModeEnabled::Id, false), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::HDRModeEnabled::Id, hdrModeEnabled), CHIP_NO_ERROR);
    EXPECT_FALSE(hdrModeEnabled);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadSupportedStreamUsages)
{
    Attributes::SupportedStreamUsages::TypeInfo::DecodableType supportedStreamUsages;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SupportedStreamUsages::Id, supportedStreamUsages), CHIP_NO_ERROR);
    auto it       = supportedStreamUsages.begin();
    uint8_t count = 0;
    while (it.Next())
    {
        ++count;
    }
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(count, 2u);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadAllocatedVideoStreams)
{
    Attributes::AllocatedVideoStreams::TypeInfo::DecodableType allocatedVideoStreams;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::AllocatedVideoStreams::Id, allocatedVideoStreams), CHIP_NO_ERROR);
    auto it       = allocatedVideoStreams.begin();
    uint8_t count = 0;
    while (it.Next())
    {
        ++count;
    }
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0u);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadAllocatedAudioStreams)
{
    Attributes::AllocatedAudioStreams::TypeInfo::DecodableType allocatedAudioStreams;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::AllocatedAudioStreams::Id, allocatedAudioStreams), CHIP_NO_ERROR);
    auto it       = allocatedAudioStreams.begin();
    uint8_t count = 0;
    while (it.Next())
    {
        ++count;
    }
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0u);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadAllocatedSnapshotStreams)
{
    Attributes::AllocatedSnapshotStreams::TypeInfo::DecodableType allocatedSnapshotStreams;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::AllocatedSnapshotStreams::Id, allocatedSnapshotStreams), CHIP_NO_ERROR);
    auto it       = allocatedSnapshotStreams.begin();
    uint8_t count = 0;
    while (it.Next())
    {
        ++count;
    }
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0u);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadStreamUsagePriorities)
{
    Attributes::StreamUsagePriorities::TypeInfo::DecodableType streamUsagePriorities;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::StreamUsagePriorities::Id, streamUsagePriorities), CHIP_NO_ERROR);
    auto it       = streamUsagePriorities.begin();
    uint8_t count = 0;
    while (it.Next())
    {
        ++count;
    }
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(count, 2u);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadWriteSoftRecordingPrivacyModeEnabled)
{
    bool softRecordingPrivacyModeEnabled = false;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SoftRecordingPrivacyModeEnabled::Id, softRecordingPrivacyModeEnabled),
              CHIP_NO_ERROR);
    EXPECT_TRUE(softRecordingPrivacyModeEnabled);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SoftRecordingPrivacyModeEnabled::Id, false), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SoftRecordingPrivacyModeEnabled::Id, softRecordingPrivacyModeEnabled),
              CHIP_NO_ERROR);
    EXPECT_FALSE(softRecordingPrivacyModeEnabled);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SoftRecordingPrivacyModeEnabled::Id, true), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SoftRecordingPrivacyModeEnabled::Id, softRecordingPrivacyModeEnabled),
              CHIP_NO_ERROR);
    EXPECT_TRUE(softRecordingPrivacyModeEnabled);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadWriteSoftLivestreamPrivacyModeEnabled)
{
    bool softLivestreamPrivacyModeEnabled = false;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SoftLivestreamPrivacyModeEnabled::Id, softLivestreamPrivacyModeEnabled),
              CHIP_NO_ERROR);
    EXPECT_TRUE(softLivestreamPrivacyModeEnabled);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SoftLivestreamPrivacyModeEnabled::Id, false), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SoftLivestreamPrivacyModeEnabled::Id, softLivestreamPrivacyModeEnabled),
              CHIP_NO_ERROR);
    EXPECT_FALSE(softLivestreamPrivacyModeEnabled);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SoftLivestreamPrivacyModeEnabled::Id, true), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SoftLivestreamPrivacyModeEnabled::Id, softLivestreamPrivacyModeEnabled),
              CHIP_NO_ERROR);
    EXPECT_TRUE(softLivestreamPrivacyModeEnabled);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadHardPrivacyModeOn)
{
    bool hardPrivacyModeOn = false;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::HardPrivacyModeOn::Id, hardPrivacyModeOn), CHIP_NO_ERROR);
    EXPECT_TRUE(hardPrivacyModeOn);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadWriteNightVision)
{
    TriStateAutoEnum nightVision = TriStateAutoEnum::kOff;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::NightVision::Id, nightVision), CHIP_NO_ERROR);
    EXPECT_EQ(nightVision, TriStateAutoEnum::kAuto); // Default should be Auto

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::NightVision::Id, TriStateAutoEnum::kOn), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::NightVision::Id, nightVision), CHIP_NO_ERROR);
    EXPECT_EQ(nightVision, TriStateAutoEnum::kOn);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::NightVision::Id, TriStateAutoEnum::kOff), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::NightVision::Id, nightVision), CHIP_NO_ERROR);

    EXPECT_EQ(nightVision, TriStateAutoEnum::kOff);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadWriteViewport)
{
    Attributes::Viewport::TypeInfo::Type viewport = { 0, 0, 0, 0 };
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::Viewport::Id, viewport), CHIP_NO_ERROR);
    EXPECT_EQ(viewport.x1, 0);
    EXPECT_EQ(viewport.y1, 0);
    EXPECT_EQ(viewport.x2, 1920);
    EXPECT_EQ(viewport.y2, 1080);

    // Attempt to write an invalid value (too small)
    Attributes::Viewport::TypeInfo::Type invalidViewport = { 10, 20, 640, 480 };
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::Viewport::Id, invalidViewport), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Read again to verify no change
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::Viewport::Id, viewport), CHIP_NO_ERROR);
    EXPECT_EQ(viewport.x1, 0);
    EXPECT_EQ(viewport.y1, 0);
    EXPECT_EQ(viewport.x2, 1920);
    EXPECT_EQ(viewport.y2, 1080);

    // Write a valid new value
    Attributes::Viewport::TypeInfo::Type newViewport = { 0, 0, 1280, 720 };
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::Viewport::Id, newViewport), CHIP_NO_ERROR);

    // Read again to verify
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::Viewport::Id, viewport), CHIP_NO_ERROR);
    EXPECT_EQ(viewport.x1, 0);
    EXPECT_EQ(viewport.y1, 0);
    EXPECT_EQ(viewport.x2, 1280);
    EXPECT_EQ(viewport.y2, 720);

    // Write back the original value
    Attributes::Viewport::TypeInfo::Type originalViewport = { 0, 0, 1920, 1080 };
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::Viewport::Id, originalViewport), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::Viewport::Id, viewport), CHIP_NO_ERROR);
    EXPECT_EQ(viewport.x1, 0);
    EXPECT_EQ(viewport.y1, 0);
    EXPECT_EQ(viewport.x2, 1920);
    EXPECT_EQ(viewport.y2, 1080);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadWriteSpeakerMuted)
{
    bool speakerMuted = false;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SpeakerMuted::Id, speakerMuted), CHIP_NO_ERROR);
    EXPECT_TRUE(speakerMuted);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SpeakerMuted::Id, false), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SpeakerMuted::Id, speakerMuted), CHIP_NO_ERROR);
    EXPECT_FALSE(speakerMuted);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SpeakerMuted::Id, true), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SpeakerMuted::Id, speakerMuted), CHIP_NO_ERROR);
    EXPECT_TRUE(speakerMuted);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadWriteSpeakerVolumeLevel)
{
    uint8_t speakerVolumeLevel = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SpeakerVolumeLevel::Id, speakerVolumeLevel), CHIP_NO_ERROR);
    EXPECT_EQ(speakerVolumeLevel, 1);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SpeakerVolumeLevel::Id, (uint8_t) 100), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SpeakerVolumeLevel::Id, speakerVolumeLevel), CHIP_NO_ERROR);
    EXPECT_EQ(speakerVolumeLevel, 100);

    // Test boundaries
    uint8_t minLevel = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SpeakerMinLevel::Id, minLevel), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SpeakerVolumeLevel::Id, minLevel), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SpeakerVolumeLevel::Id, speakerVolumeLevel), CHIP_NO_ERROR);
    EXPECT_EQ(speakerVolumeLevel, minLevel);

    uint8_t maxLevel = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SpeakerMaxLevel::Id, maxLevel), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SpeakerVolumeLevel::Id, maxLevel), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SpeakerVolumeLevel::Id, speakerVolumeLevel), CHIP_NO_ERROR);
    EXPECT_EQ(speakerVolumeLevel, maxLevel);

    // Test out of bounds
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SpeakerVolumeLevel::Id, (uint8_t) (0)),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SpeakerVolumeLevel::Id, (uint8_t) (255)),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SpeakerVolumeLevel::Id, (uint8_t) 1), CHIP_NO_ERROR); // Restore default
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadSpeakerMaxLevel)
{
    uint8_t speakerMaxLevel = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SpeakerMaxLevel::Id, speakerMaxLevel), CHIP_NO_ERROR);
    EXPECT_EQ(speakerMaxLevel, 254);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadSpeakerMinLevel)
{
    uint8_t speakerMinLevel = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SpeakerMinLevel::Id, speakerMinLevel), CHIP_NO_ERROR);
    EXPECT_EQ(speakerMinLevel, 1);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadWriteMicrophoneMuted)
{
    bool microphoneMuted = false;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MicrophoneMuted::Id, microphoneMuted), CHIP_NO_ERROR);
    EXPECT_TRUE(microphoneMuted);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::MicrophoneMuted::Id, false), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MicrophoneMuted::Id, microphoneMuted), CHIP_NO_ERROR);
    EXPECT_FALSE(microphoneMuted);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::MicrophoneMuted::Id, true), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MicrophoneMuted::Id, microphoneMuted), CHIP_NO_ERROR);
    EXPECT_TRUE(microphoneMuted);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadWriteMicrophoneVolumeLevel)
{
    uint8_t microphoneVolumeLevel = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MicrophoneVolumeLevel::Id, microphoneVolumeLevel), CHIP_NO_ERROR);
    EXPECT_EQ(microphoneVolumeLevel, 1);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::MicrophoneVolumeLevel::Id, (uint8_t) 50), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MicrophoneVolumeLevel::Id, microphoneVolumeLevel), CHIP_NO_ERROR);
    EXPECT_EQ(microphoneVolumeLevel, 50);

    // Test boundaries
    uint8_t minLevel = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MicrophoneMinLevel::Id, minLevel), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::MicrophoneVolumeLevel::Id, minLevel), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MicrophoneVolumeLevel::Id, microphoneVolumeLevel), CHIP_NO_ERROR);
    EXPECT_EQ(microphoneVolumeLevel, minLevel);

    uint8_t maxLevel = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MicrophoneMaxLevel::Id, maxLevel), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::MicrophoneVolumeLevel::Id, maxLevel), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MicrophoneVolumeLevel::Id, microphoneVolumeLevel), CHIP_NO_ERROR);
    EXPECT_EQ(microphoneVolumeLevel, maxLevel);

    // Test out of bounds
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::MicrophoneVolumeLevel::Id, (uint8_t) (0)),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::MicrophoneVolumeLevel::Id, (uint8_t) (255)),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::MicrophoneVolumeLevel::Id, (uint8_t) 1), CHIP_NO_ERROR); // Restore default
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadMicrophoneMaxLevel)
{
    uint8_t microphoneMaxLevel = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MicrophoneMaxLevel::Id, microphoneMaxLevel), CHIP_NO_ERROR);
    EXPECT_EQ(microphoneMaxLevel, 254);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadMicrophoneMinLevel)
{
    uint8_t microphoneMinLevel = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MicrophoneMinLevel::Id, microphoneMinLevel), CHIP_NO_ERROR);
    EXPECT_EQ(microphoneMinLevel, 1);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadMicrophoneAGCEnabled)
{
    bool microphoneAGCEnabled = false;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MicrophoneAGCEnabled::Id, microphoneAGCEnabled), CHIP_NO_ERROR);
    EXPECT_TRUE(microphoneAGCEnabled);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadWriteImageRotation)
{
    uint16_t imageRotation = 1;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ImageRotation::Id, imageRotation), CHIP_NO_ERROR);
    EXPECT_EQ(imageRotation, 0);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::ImageRotation::Id, (uint16_t) 90), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ImageRotation::Id, imageRotation), CHIP_NO_ERROR);
    EXPECT_EQ(imageRotation, 90);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::ImageRotation::Id, (uint16_t) 180), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ImageRotation::Id, imageRotation), CHIP_NO_ERROR);
    EXPECT_EQ(imageRotation, 180);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::ImageRotation::Id, (uint16_t) 270), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ImageRotation::Id, imageRotation), CHIP_NO_ERROR);
    EXPECT_EQ(imageRotation, 270);

    // Test invalid value
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::ImageRotation::Id, (uint16_t) 360), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::ImageRotation::Id, (uint16_t) 0), CHIP_NO_ERROR); // Restore default
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadWriteImageFlipHorizontal)
{
    bool imageFlipHorizontal = true;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ImageFlipHorizontal::Id, imageFlipHorizontal), CHIP_NO_ERROR);
    EXPECT_FALSE(imageFlipHorizontal);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::ImageFlipHorizontal::Id, true), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ImageFlipHorizontal::Id, imageFlipHorizontal), CHIP_NO_ERROR);
    EXPECT_TRUE(imageFlipHorizontal);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::ImageFlipHorizontal::Id, false), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ImageFlipHorizontal::Id, imageFlipHorizontal), CHIP_NO_ERROR);
    EXPECT_FALSE(imageFlipHorizontal);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadWriteImageFlipVertical)
{
    bool imageFlipVertical = true;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ImageFlipVertical::Id, imageFlipVertical), CHIP_NO_ERROR);
    EXPECT_FALSE(imageFlipVertical);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::ImageFlipVertical::Id, true), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ImageFlipVertical::Id, imageFlipVertical), CHIP_NO_ERROR);
    EXPECT_TRUE(imageFlipVertical);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::ImageFlipVertical::Id, false), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ImageFlipVertical::Id, imageFlipVertical), CHIP_NO_ERROR);
    EXPECT_FALSE(imageFlipVertical);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadWriteStatusLightEnabled)
{
    bool statusLightEnabled = false;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::StatusLightEnabled::Id, statusLightEnabled), CHIP_NO_ERROR);
    EXPECT_TRUE(statusLightEnabled);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::StatusLightEnabled::Id, false), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::StatusLightEnabled::Id, statusLightEnabled), CHIP_NO_ERROR);
    EXPECT_FALSE(statusLightEnabled);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::StatusLightEnabled::Id, true), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::StatusLightEnabled::Id, statusLightEnabled), CHIP_NO_ERROR);
    EXPECT_TRUE(statusLightEnabled);
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadWriteStatusLightBrightness)
{
    Attributes::StatusLightBrightness::TypeInfo::Type statusLightBrightness = Globals::ThreeLevelAutoEnum::kAuto;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::StatusLightBrightness::Id, statusLightBrightness), CHIP_NO_ERROR);
    EXPECT_EQ(statusLightBrightness, Globals::ThreeLevelAutoEnum::kMedium);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::StatusLightBrightness::Id, Globals::ThreeLevelAutoEnum::kLow),
              CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::StatusLightBrightness::Id, statusLightBrightness), CHIP_NO_ERROR);
    EXPECT_EQ(statusLightBrightness, Globals::ThreeLevelAutoEnum::kLow);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::StatusLightBrightness::Id, Globals::ThreeLevelAutoEnum::kHigh),
              CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::StatusLightBrightness::Id, statusLightBrightness), CHIP_NO_ERROR);
    EXPECT_EQ(statusLightBrightness, Globals::ThreeLevelAutoEnum::kHigh);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::StatusLightBrightness::Id, Globals::ThreeLevelAutoEnum::kAuto),
              CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::StatusLightBrightness::Id, statusLightBrightness), CHIP_NO_ERROR);
    EXPECT_EQ(statusLightBrightness, Globals::ThreeLevelAutoEnum::kAuto);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::StatusLightBrightness::Id, Globals::ThreeLevelAutoEnum::kMedium),
              CHIP_NO_ERROR); // Restore default
}

TEST_F(TestCameraAVStreamManagementCluster, TestAudioStreamAllocateCommand)
{
    using Request  = Commands::AudioStreamAllocate::Type;
    using Response = Commands::AudioStreamAllocateResponse::DecodableType;

    mAudioStreams.clear();

    Request request;

    // Happy path
    request.streamUsage  = StreamUsageEnum::kLiveView;
    request.audioCodec   = AudioCodecEnum::kOpus;
    request.channelCount = 2;
    request.sampleRate   = 48000;
    request.bitRate      = 128000;
    request.bitDepth     = 24;

    auto result = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.response->audioStreamID, 1);

    // channelCount out of bounds
    request.channelCount = 0;
    result               = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);

    request.channelCount = 9;
    result               = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    request.channelCount = 2; // Restore

    // sampleRate 0
    request.sampleRate = 0;
    result             = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    request.sampleRate = 48000; // Restore

    // bitRate 0
    request.bitRate = 0;
    result          = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    request.bitRate = 128000; // Restore

    // Invalid bitDepth
    request.bitDepth = 12;
    result           = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    request.bitDepth = 24; // Restore

    // streamUsage not supported by test fixture priorities
    request.streamUsage = StreamUsageEnum::kAnalysis;
    result              = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidInState);
    request.streamUsage = StreamUsageEnum::kLiveView; // Restore

    // Attempt to allocate a second stream, expecting ResourceExhausted
    result = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ResourceExhausted);
}

TEST_F(TestCameraAVStreamManagementCluster, TestAudioStreamAllocateCommandUnsupportedBitDepth)
{
    using Request  = Commands::AudioStreamAllocate::Type;
    using Response = Commands::AudioStreamAllocateResponse::DecodableType;

    mAudioStreams.clear();

    Request request;

    // Unsupported bitDepth
    request.streamUsage  = StreamUsageEnum::kLiveView;
    request.audioCodec   = AudioCodecEnum::kOpus;
    request.channelCount = 2;
    request.sampleRate   = 48000;
    request.bitRate      = 128000;
    request.bitDepth     = 8; // Unsupported bitDepth

    auto result = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::DynamicConstraintError);
}

TEST_F(TestCameraAVStreamManagementCluster, TestAudioStreamDeallocateCommand)
{
    using AllocateRequest   = Commands::AudioStreamAllocate::Type;
    using AllocateResponse  = Commands::AudioStreamAllocateResponse::DecodableType;
    using DeallocateRequest = Commands::AudioStreamDeallocate::Type;

    mAudioStreams.clear();

    // First, allocate a stream
    AllocateRequest allocRequest;
    allocRequest.streamUsage  = StreamUsageEnum::kLiveView;
    allocRequest.audioCodec   = AudioCodecEnum::kOpus;
    allocRequest.channelCount = 2;
    allocRequest.sampleRate   = 48000;
    allocRequest.bitRate      = 128000;
    allocRequest.bitDepth     = 24;

    auto allocResult = mClusterTester.Invoke<AllocateRequest, AllocateResponse>(allocRequest);
    ASSERT_TRUE(allocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_TRUE(allocResult.IsSuccess());
    ASSERT_TRUE(allocResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    uint16_t streamId = allocResult.response->audioStreamID;
    EXPECT_EQ(mServer.GetAllocatedAudioStreams().size(), 1u);

    // Deallocate the stream
    DeallocateRequest deallocRequest;
    deallocRequest.audioStreamID = streamId;
    auto deallocResult           = mClusterTester.Invoke<DeallocateRequest, DataModel::NullObjectType>(deallocRequest);
    ASSERT_TRUE(deallocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(deallocResult.IsSuccess());
    EXPECT_EQ(mServer.GetAllocatedAudioStreams().size(), 0u);

    // Attempt to deallocate again, should fail
    deallocResult = mClusterTester.Invoke<DeallocateRequest, DataModel::NullObjectType>(deallocRequest);
    ASSERT_TRUE(deallocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(deallocResult.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);

    // Attempt to deallocate a non-existent ID
    deallocRequest.audioStreamID = 999;
    deallocResult                = mClusterTester.Invoke<DeallocateRequest, DataModel::NullObjectType>(deallocRequest);
    ASSERT_TRUE(deallocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(deallocResult.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);
}

TEST_F(TestCameraAVStreamManagementCluster, TestVideoStreamAllocateCommand)
{
    using Request  = Commands::VideoStreamAllocate::Type;
    using Response = Commands::VideoStreamAllocateResponse::DecodableType;

    mVideoStreams.clear();

    Request request;

    // Happy path
    request.streamUsage      = StreamUsageEnum::kLiveView;
    request.videoCodec       = VideoCodecEnum::kH264;
    request.minFrameRate     = 30;
    request.maxFrameRate     = GetVideoSensorParams().maxFPS;
    request.minResolution    = { 640, 480 };
    request.maxResolution    = { 1280, 720 };
    request.minBitRate       = 10000;
    request.maxBitRate       = 10000;
    request.keyFrameInterval = 4;
    request.watermarkEnabled = chip::MakeOptional(false);

    auto result = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.response->videoStreamID, 1);
    EXPECT_EQ(mServer.GetAllocatedVideoStreams().size(), 1u);

    // Invalid streamUsage
    request.streamUsage = StreamUsageEnum::kAnalysis;
    result              = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidInState);
    request.streamUsage = StreamUsageEnum::kLiveView; // Restore

    // minFrameRate > maxFrameRate
    request.minFrameRate = 121;
    result               = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    request.minFrameRate = 30; // Restore

    // minResolution > maxResolution
    request.minResolution = { 1281, 720 };
    result                = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    request.minResolution = { 640, 480 }; // Restore

    // minBitRate > maxBitRate
    request.minBitRate = 10001;
    result             = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    request.minBitRate = 10000; // Restore

    // keyFrameInterval 65501
    request.keyFrameInterval = 65501;
    result                   = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    request.keyFrameInterval = 4; // Restore

    // Unsupported videoCodec
    request.videoCodec = VideoCodecEnum::kHevc;
    result             = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::DynamicConstraintError);
    request.videoCodec = VideoCodecEnum::kH264; // Restore

    // Attempt to allocate a second stream, expecting ResourceExhausted
    result = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ResourceExhausted);
}

TEST_F(TestCameraAVStreamManagementCluster, TestVideoStreamDeallocateCommand)
{
    using AllocateRequest   = Commands::VideoStreamAllocate::Type;
    using AllocateResponse  = Commands::VideoStreamAllocateResponse::DecodableType;
    using DeallocateRequest = Commands::VideoStreamDeallocate::Type;

    mVideoStreams.clear();

    // First, allocate a stream
    AllocateRequest allocRequest;
    allocRequest.streamUsage      = StreamUsageEnum::kLiveView;
    allocRequest.videoCodec       = VideoCodecEnum::kH264;
    allocRequest.minFrameRate     = 30;
    allocRequest.maxFrameRate     = 120;
    allocRequest.minResolution    = { 640, 480 };
    allocRequest.maxResolution    = { 1280, 720 };
    allocRequest.minBitRate       = 10000;
    allocRequest.maxBitRate       = 10000;
    allocRequest.keyFrameInterval = 4;
    allocRequest.watermarkEnabled = chip::MakeOptional(false);

    auto allocResult = mClusterTester.Invoke<AllocateRequest, AllocateResponse>(allocRequest);
    ASSERT_TRUE(allocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_TRUE(allocResult.IsSuccess());
    ASSERT_TRUE(allocResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    uint16_t streamId = allocResult.response->videoStreamID;
    EXPECT_EQ(mServer.GetAllocatedVideoStreams().size(), 1u);

    // Deallocate the stream
    DeallocateRequest deallocRequest;
    deallocRequest.videoStreamID = streamId;
    auto deallocResult           = mClusterTester.Invoke<DeallocateRequest, DataModel::NullObjectType>(deallocRequest);
    ASSERT_TRUE(deallocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(deallocResult.IsSuccess());
    EXPECT_EQ(mServer.GetAllocatedVideoStreams().size(), 0u);

    // Attempt to deallocate again, should fail
    deallocResult = mClusterTester.Invoke<DeallocateRequest, DataModel::NullObjectType>(deallocRequest);
    ASSERT_TRUE(deallocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(deallocResult.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);

    // Attempt to deallocate a non-existent ID
    deallocRequest.videoStreamID = 999;
    deallocResult                = mClusterTester.Invoke<DeallocateRequest, DataModel::NullObjectType>(deallocRequest);
    ASSERT_TRUE(deallocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(deallocResult.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);
}

TEST_F(TestCameraAVStreamManagementCluster, TestVideoStreamModifyCommand)
{
    using AllocateRequest  = Commands::VideoStreamAllocate::Type;
    using AllocateResponse = Commands::VideoStreamAllocateResponse::DecodableType;
    using ModifyRequest    = Commands::VideoStreamModify::Type;

    mVideoStreams.clear();

    // First, allocate a stream
    AllocateRequest allocRequest;
    allocRequest.streamUsage      = StreamUsageEnum::kLiveView;
    allocRequest.videoCodec       = VideoCodecEnum::kH264;
    allocRequest.minFrameRate     = 30;
    allocRequest.maxFrameRate     = 120;
    allocRequest.minResolution    = { 640, 480 };
    allocRequest.maxResolution    = { 1280, 720 };
    allocRequest.minBitRate       = 10000;
    allocRequest.maxBitRate       = 10000;
    allocRequest.keyFrameInterval = 4;
    allocRequest.watermarkEnabled = chip::MakeOptional(false);

    auto allocResult = mClusterTester.Invoke<AllocateRequest, AllocateResponse>(allocRequest);
    ASSERT_TRUE(allocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_TRUE(allocResult.IsSuccess());
    ASSERT_TRUE(allocResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    uint16_t streamId = allocResult.response->videoStreamID;
    EXPECT_EQ(mServer.GetAllocatedVideoStreams().size(), 1u);

    // Modify the stream
    ModifyRequest modifyRequest;
    modifyRequest.videoStreamID    = streamId;
    modifyRequest.watermarkEnabled = chip::MakeOptional(true);

    auto modifyResult = mClusterTester.Invoke<ModifyRequest, DataModel::NullObjectType>(modifyRequest);
    ASSERT_TRUE(modifyResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(modifyResult.IsSuccess());

    // Verify the changes
    Attributes::AllocatedVideoStreams::TypeInfo::DecodableType allocatedVideoStreams;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::AllocatedVideoStreams::Id, allocatedVideoStreams), CHIP_NO_ERROR);
    auto it = allocatedVideoStreams.begin();
    ASSERT_TRUE(it.Next());
    const auto & stream = it.GetValue();
    EXPECT_EQ(stream.videoStreamID, streamId);
    EXPECT_TRUE(stream.watermarkEnabled.Value());
    EXPECT_FALSE(it.Next());
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);

    // Modify only watermark
    modifyRequest.watermarkEnabled = chip::MakeOptional(false);
    modifyRequest.OSDEnabled       = chip::Optional<bool>::Missing();
    modifyResult                   = mClusterTester.Invoke<ModifyRequest, DataModel::NullObjectType>(modifyRequest);
    ASSERT_TRUE(modifyResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(modifyResult.IsSuccess());

    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::AllocatedVideoStreams::Id, allocatedVideoStreams), CHIP_NO_ERROR);
    it = allocatedVideoStreams.begin();
    ASSERT_TRUE(it.Next());
    const auto & stream2 = it.GetValue();
    EXPECT_EQ(stream2.videoStreamID, streamId);
    EXPECT_FALSE(stream2.watermarkEnabled.Value());
    EXPECT_FALSE(it.Next());
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);

    // Attempt to modify a non-existent ID
    modifyRequest.videoStreamID = 999;
    modifyResult                = mClusterTester.Invoke<ModifyRequest, DataModel::NullObjectType>(modifyRequest);
    ASSERT_TRUE(modifyResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(modifyResult.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);
}

TEST_F(TestCameraAVStreamManagementCluster, TestSnapshotStreamAllocateCommand)
{
    using Request  = Commands::SnapshotStreamAllocate::Type;
    using Response = Commands::SnapshotStreamAllocateResponse::DecodableType;

    mSnapshotStreams.clear();

    Request request;

    // Happy path
    request.imageCodec       = ImageCodecEnum::kJpeg;
    request.maxFrameRate     = 30;
    request.minResolution    = { 640, 480 };
    request.maxResolution    = { 1280, 720 };
    request.quality          = 80;
    request.watermarkEnabled = chip::MakeOptional(false);

    auto result = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.response->snapshotStreamID, 1);
    EXPECT_EQ(mServer.GetAllocatedSnapshotStreams().size(), 1u);

    // maxFrameRate 0
    request.maxFrameRate = 0;
    result               = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    request.maxFrameRate = 30; // Restore

    // minResolution > maxResolution
    request.minResolution = { 1281, 720 };
    result                = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    request.minResolution = { 640, 480 }; // Restore

    // quality 0
    request.quality = 0;
    result          = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    request.quality = 80; // Restore

    // quality 101
    request.quality = 101;
    result          = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    request.quality = 80; // Restore

    // Unsupported maxFrameRate
    request.maxFrameRate = 200;
    result               = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::DynamicConstraintError);
    request.maxFrameRate = 30; // Restore

    // Attempt to allocate a second stream, expecting ResourceExhausted
    result = mClusterTester.Invoke<Request, Response>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ResourceExhausted);
}

TEST_F(TestCameraAVStreamManagementCluster, TestSnapshotStreamDeallocateCommand)
{
    using AllocateRequest   = Commands::SnapshotStreamAllocate::Type;
    using AllocateResponse  = Commands::SnapshotStreamAllocateResponse::DecodableType;
    using DeallocateRequest = Commands::SnapshotStreamDeallocate::Type;

    mSnapshotStreams.clear();

    // First, allocate a stream
    AllocateRequest allocRequest;
    allocRequest.imageCodec       = ImageCodecEnum::kJpeg;
    allocRequest.maxFrameRate     = 30;
    allocRequest.minResolution    = { 640, 480 };
    allocRequest.maxResolution    = { 640, 480 };
    allocRequest.quality          = 80;
    allocRequest.watermarkEnabled = chip::MakeOptional(false);

    auto allocResult = mClusterTester.Invoke<AllocateRequest, AllocateResponse>(allocRequest);
    ASSERT_TRUE(allocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_TRUE(allocResult.IsSuccess());
    ASSERT_TRUE(allocResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    uint16_t streamId = allocResult.response->snapshotStreamID;
    EXPECT_EQ(mServer.GetAllocatedSnapshotStreams().size(), 1u);

    // Deallocate the stream
    DeallocateRequest deallocRequest;
    deallocRequest.snapshotStreamID = streamId;
    auto deallocResult              = mClusterTester.Invoke<DeallocateRequest, DataModel::NullObjectType>(deallocRequest);
    ASSERT_TRUE(deallocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(deallocResult.IsSuccess());
    EXPECT_EQ(mServer.GetAllocatedSnapshotStreams().size(), 0u);

    // Attempt to deallocate again, should fail
    deallocResult = mClusterTester.Invoke<DeallocateRequest, DataModel::NullObjectType>(deallocRequest);
    ASSERT_TRUE(deallocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(deallocResult.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);

    // Attempt to deallocate a non-existent ID
    deallocRequest.snapshotStreamID = 999;
    deallocResult                   = mClusterTester.Invoke<DeallocateRequest, DataModel::NullObjectType>(deallocRequest);
    ASSERT_TRUE(deallocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(deallocResult.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);
}

TEST_F(TestCameraAVStreamManagementCluster, TestSnapshotStreamModifyCommand)
{
    using AllocateRequest  = Commands::SnapshotStreamAllocate::Type;
    using AllocateResponse = Commands::SnapshotStreamAllocateResponse::DecodableType;
    using ModifyRequest    = Commands::SnapshotStreamModify::Type;

    mSnapshotStreams.clear();

    // First, allocate a stream
    AllocateRequest allocRequest;
    allocRequest.imageCodec       = ImageCodecEnum::kJpeg;
    allocRequest.maxFrameRate     = 30;
    allocRequest.minResolution    = { 1280, 720 };
    allocRequest.maxResolution    = { 1280, 720 };
    allocRequest.quality          = 80;
    allocRequest.watermarkEnabled = chip::MakeOptional(false);

    auto allocResult = mClusterTester.Invoke<AllocateRequest, AllocateResponse>(allocRequest);
    ASSERT_TRUE(allocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_TRUE(allocResult.IsSuccess());
    ASSERT_TRUE(allocResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    uint16_t streamId = allocResult.response->snapshotStreamID;
    EXPECT_EQ(mServer.GetAllocatedSnapshotStreams().size(), 1u);

    // Modify the stream
    ModifyRequest modifyRequest;
    modifyRequest.snapshotStreamID = streamId;
    modifyRequest.watermarkEnabled = chip::MakeOptional(true);

    auto modifyResult = mClusterTester.Invoke<ModifyRequest, DataModel::NullObjectType>(modifyRequest);
    ASSERT_TRUE(modifyResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(modifyResult.IsSuccess());

    // Verify the changes
    Attributes::AllocatedSnapshotStreams::TypeInfo::DecodableType allocatedSnapshotStreams;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::AllocatedSnapshotStreams::Id, allocatedSnapshotStreams), CHIP_NO_ERROR);
    auto it = allocatedSnapshotStreams.begin();
    ASSERT_TRUE(it.Next());
    const auto & stream = it.GetValue();
    EXPECT_EQ(stream.snapshotStreamID, streamId);
    EXPECT_TRUE(stream.watermarkEnabled.Value());
    EXPECT_FALSE(it.Next());
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);

    // Modify watermark
    modifyRequest.watermarkEnabled = chip::MakeOptional(false);
    modifyRequest.OSDEnabled       = chip::Optional<bool>::Missing();
    modifyResult                   = mClusterTester.Invoke<ModifyRequest, DataModel::NullObjectType>(modifyRequest);
    ASSERT_TRUE(modifyResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(modifyResult.IsSuccess());

    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::AllocatedSnapshotStreams::Id, allocatedSnapshotStreams), CHIP_NO_ERROR);
    it = allocatedSnapshotStreams.begin();
    ASSERT_TRUE(it.Next());
    const auto & stream2 = it.GetValue();
    EXPECT_EQ(stream2.snapshotStreamID, streamId);
    EXPECT_FALSE(stream2.watermarkEnabled.Value());
    EXPECT_FALSE(it.Next());
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);

    // Attempt to modify a non-existent ID
    modifyRequest.snapshotStreamID = 999;
    modifyResult                   = mClusterTester.Invoke<ModifyRequest, DataModel::NullObjectType>(modifyRequest);
    ASSERT_TRUE(modifyResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(modifyResult.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);
}

TEST_F(TestCameraAVStreamManagementCluster, TestSetStreamPrioritiesCommand)
{
    using Request = Commands::SetStreamPriorities::Type;

    mVideoStreams.clear();
    mAudioStreams.clear();
    mSnapshotStreams.clear();

    Request request;
    std::vector<StreamUsageEnum> priorities;

    // Happy path
    priorities.push_back(StreamUsageEnum::kRecording);
    priorities.push_back(StreamUsageEnum::kLiveView);
    request.streamPriorities = DataModel::List<const StreamUsageEnum>(priorities.data(), priorities.size());
    auto result              = mClusterTester.Invoke<Request, DataModel::NullObjectType>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(result.IsSuccess());

    Attributes::StreamUsagePriorities::TypeInfo::DecodableType readPriorities;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::StreamUsagePriorities::Id, readPriorities), CHIP_NO_ERROR);
    auto it = readPriorities.begin();
    ASSERT_TRUE(it.Next());
    EXPECT_EQ(it.GetValue(), StreamUsageEnum::kRecording);
    ASSERT_TRUE(it.Next());
    EXPECT_EQ(it.GetValue(), StreamUsageEnum::kLiveView);
    EXPECT_FALSE(it.Next());
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);

    // Invalid enum value
    priorities.clear();
    priorities.push_back(static_cast<StreamUsageEnum>(0xFF));
    request.streamPriorities = DataModel::List<const StreamUsageEnum>(priorities.data(), priorities.size());
    result                   = mClusterTester.Invoke<Request, DataModel::NullObjectType>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidCommand);

    // Duplicate enum value
    priorities.clear();
    priorities.push_back(StreamUsageEnum::kLiveView);
    priorities.push_back(StreamUsageEnum::kLiveView);
    request.streamPriorities = DataModel::List<const StreamUsageEnum>(priorities.data(), priorities.size());
    result                   = mClusterTester.Invoke<Request, DataModel::NullObjectType>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::AlreadyExists);

    // Unsupported enum value
    priorities.clear();
    priorities.push_back(StreamUsageEnum::kAnalysis);
    request.streamPriorities = DataModel::List<const StreamUsageEnum>(priorities.data(), priorities.size());
    result                   = mClusterTester.Invoke<Request, DataModel::NullObjectType>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::DynamicConstraintError);

    // Invalid state - stream allocated
    {
        using AllocateRequest  = Commands::VideoStreamAllocate::Type;
        using AllocateResponse = Commands::VideoStreamAllocateResponse::DecodableType;

        mVideoStreams.clear();

        AllocateRequest allocRequest;
        allocRequest.streamUsage      = StreamUsageEnum::kLiveView;
        allocRequest.videoCodec       = VideoCodecEnum::kH264;
        allocRequest.minFrameRate     = 30;
        allocRequest.maxFrameRate     = 120;
        allocRequest.minResolution    = { 640, 480 };
        allocRequest.maxResolution    = { 1280, 720 };
        allocRequest.minBitRate       = 10000;
        allocRequest.maxBitRate       = 10000;
        allocRequest.keyFrameInterval = 4;
        allocRequest.watermarkEnabled = chip::MakeOptional(false);

        auto allocResult = mClusterTester.Invoke<AllocateRequest, AllocateResponse>(allocRequest);
        ASSERT_TRUE(allocResult.status.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        ASSERT_TRUE(allocResult.IsSuccess());
        EXPECT_EQ(mServer.GetAllocatedVideoStreams().size(), 1u);
    }

    priorities.clear();
    priorities.push_back(StreamUsageEnum::kLiveView);
    request.streamPriorities = DataModel::List<const StreamUsageEnum>(priorities.data(), priorities.size());
    result                   = mClusterTester.Invoke<Request, DataModel::NullObjectType>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidInState);

    // Clean up
    {
        using DeallocateRequest = Commands::VideoStreamDeallocate::Type;
        DeallocateRequest deallocRequest;
        deallocRequest.videoStreamID = 1;
        auto deallocResult           = mClusterTester.Invoke<DeallocateRequest, DataModel::NullObjectType>(deallocRequest);
        ASSERT_TRUE(deallocResult.status.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_TRUE(deallocResult.IsSuccess());
        EXPECT_EQ(mServer.GetAllocatedVideoStreams().size(), 0u);

        mVideoStreams.clear();
    }
}

TEST_F(TestCameraAVStreamManagementCluster, TestCaptureSnapshotCommand)
{
    using AllocateRequest  = Commands::SnapshotStreamAllocate::Type;
    using AllocateResponse = Commands::SnapshotStreamAllocateResponse::DecodableType;
    using CaptureRequest   = Commands::CaptureSnapshot::Type;
    using CaptureResponse  = Commands::CaptureSnapshotResponse::DecodableType;

    mSnapshotStreams.clear();

    // First, allocate a stream
    AllocateRequest allocRequest;
    allocRequest.imageCodec       = ImageCodecEnum::kJpeg;
    allocRequest.maxFrameRate     = 30;
    allocRequest.minResolution    = { 640, 480 };
    allocRequest.maxResolution    = { 1280, 720 };
    allocRequest.quality          = 80;
    allocRequest.watermarkEnabled = chip::MakeOptional(false);

    auto allocResult = mClusterTester.Invoke<AllocateRequest, AllocateResponse>(allocRequest);
    ASSERT_TRUE(allocResult.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_TRUE(allocResult.IsSuccess());
    ASSERT_TRUE(allocResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    uint16_t streamId = allocResult.response->snapshotStreamID;
    EXPECT_EQ(mServer.GetAllocatedSnapshotStreams().size(), 1u);

    // Disable privacy modes to allow capture
    EXPECT_EQ(mServer.SetSoftLivestreamPrivacyModeEnabled(false), CHIP_NO_ERROR);
    EXPECT_EQ(mServer.SetHardPrivacyModeOn(false), CHIP_NO_ERROR);

    CaptureRequest request;
    request.snapshotStreamID.SetNonNull(streamId);
    request.requestedResolution = { 640, 480 };

    auto result = mClusterTester.Invoke<CaptureRequest, CaptureResponse>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.response->imageCodec, ImageCodecEnum::kJpeg);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.response->resolution.width, 640);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.response->resolution.height, 480);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_GT(result.response->data.size(), 0u);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.response->data.data()[0], 0xFF); // Basic check on dummy data
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.response->data.data()[1], 0xD8); // Basic check on dummy data

    // Restore privacy modes
    EXPECT_EQ(mServer.SetSoftLivestreamPrivacyModeEnabled(true), CHIP_NO_ERROR);
    EXPECT_EQ(mServer.SetHardPrivacyModeOn(true), CHIP_NO_ERROR);

    // Check for InvalidInState error
    result = mClusterTester.Invoke<CaptureRequest, CaptureResponse>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::InvalidInState);

    // Check for NotFound error with invalid stream ID
    request.snapshotStreamID.SetNonNull(999);
    result = mClusterTester.Invoke<CaptureRequest, CaptureResponse>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);
}

TEST_F(TestCameraAVStreamManagementCluster, TestCaptureSnapshotCommand_NoStreamAllocated)
{
    using CaptureRequest  = Commands::CaptureSnapshot::Type;
    using CaptureResponse = Commands::CaptureSnapshotResponse::DecodableType;

    mSnapshotStreams.clear();

    // Disable privacy modes to allow capture
    EXPECT_EQ(mServer.SetSoftLivestreamPrivacyModeEnabled(false), CHIP_NO_ERROR);
    EXPECT_EQ(mServer.SetHardPrivacyModeOn(false), CHIP_NO_ERROR);

    CaptureRequest request;
    request.snapshotStreamID.SetNull();
    request.requestedResolution = { 640, 480 };

    auto result = mClusterTester.Invoke<CaptureRequest, CaptureResponse>(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);
}

} // namespace
