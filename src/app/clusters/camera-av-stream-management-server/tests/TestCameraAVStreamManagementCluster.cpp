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

// Mock delegate for testing CameraAVStreamManagement
class MockCameraAVStreamManagementDelegate : public CameraAVStreamManagementDelegate
{
public:
    MockCameraAVStreamManagementDelegate(std::vector<VideoStreamStruct> * videoStreams,
                                         std::vector<AudioStreamStruct> * audioStreams,
                                         std::vector<SnapshotStreamStruct> * snapshotStreams) :
        mAllocatedVideoStreams(videoStreams),
        mAllocatedAudioStreams(audioStreams), mAllocatedSnapshotStreams(snapshotStreams)
    {}

    Protocols::InteractionModel::Status VideoStreamAllocate(const VideoStreamStruct & allocateArgs, uint16_t & outStreamID) override
    {
        outStreamID = 1;
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
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status AudioStreamAllocate(const AudioStreamStruct & allocateArgs, uint16_t & outStreamID) override
    {
        outStreamID = 1;
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status AudioStreamDeallocate(const uint16_t streamID) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status SnapshotStreamAllocate(const SnapshotStreamAllocateArgs & allocateArgs,
                                                               uint16_t & outStreamID) override
    {
        outStreamID = 1;
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status SnapshotStreamModify(const uint16_t streamID, const Optional<bool> waterMarkEnabled,
                                                             const Optional<bool> osdEnabled) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status SnapshotStreamDeallocate(const uint16_t streamID) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    void OnStreamUsagePrioritiesChanged() override {}

    void OnAttributeChanged(AttributeId attributeId) override {}

    Protocols::InteractionModel::Status CaptureSnapshot(const DataModel::Nullable<uint16_t> streamID,
                                                        const VideoResolutionStruct & resolution,
                                                        ImageSnapshot & outImageSnapshot) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    CHIP_ERROR PersistentAttributesLoadedCallback() override { return CHIP_NO_ERROR; }

    CHIP_ERROR OnTransportAcquireAudioVideoStreams(uint16_t audioStreamID, uint16_t videoStreamID) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnTransportReleaseAudioVideoStreams(uint16_t audioStreamID, uint16_t videoStreamID) override
    {
        return CHIP_NO_ERROR;
    }

    const std::vector<VideoStreamStruct> & GetAllocatedVideoStreams() const override { return *mAllocatedVideoStreams; }

    const std::vector<AudioStreamStruct> & GetAllocatedAudioStreams() const override { return *mAllocatedAudioStreams; }

    const std::vector<SnapshotStreamStruct> & GetAllocatedSnapshotStreams() const { return *mAllocatedSnapshotStreams; }

private:
    std::vector<VideoStreamStruct> * mAllocatedVideoStreams;
    std::vector<AudioStreamStruct> * mAllocatedAudioStreams;
    std::vector<SnapshotStreamStruct> * mAllocatedSnapshotStreams;
};

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestCameraAVStreamManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

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

    static AudioCapabilitiesStruct & GetAudioCapabilities()
    {
        static std::array<AudioCodecEnum, 2> audioCodecs = { AudioCodecEnum::kOpus, AudioCodecEnum::kAacLc };
        static std::array<uint32_t, 2> sampleRates       = { 48000, 32000 }; // Sample rates in Hz
        static std::array<uint8_t, 2> bitDepths          = { 24, 32 };
        static AudioCapabilitiesStruct audioCapabilities = { 2, chip::Span<AudioCodecEnum>(audioCodecs),
                                                             chip::Span<uint32_t>(sampleRates), chip::Span<uint8_t>(bitDepths) };
        return audioCapabilities;
    }

    static std::vector<SnapshotCapabilitiesStruct> & GetSnapshotCapabilities()
    {
        static std::vector<SnapshotCapabilitiesStruct> snapshotCapabilities = {
            { { 640, 480 }, 30, ImageCodecEnum::kJpeg, false, chip::MakeOptional(static_cast<bool>(false)) },
            { { 1280, 720 }, 30, ImageCodecEnum::kJpeg, true, chip::MakeOptional(static_cast<bool>(true)) },
        };
        return snapshotCapabilities;
    }

    static std::vector<StreamUsageEnum> & GetSupportedStreamUsages()
    {
        static std::vector<StreamUsageEnum> supportedStreamUsage = { StreamUsageEnum::kLiveView, StreamUsageEnum::kRecording };
        return supportedStreamUsage;
    }

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
        mServer(mMockDelegate, kTestEndpointId,
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
        app::SetSafeAttributePersistenceProvider(&mPersistenceProvider);
        EXPECT_EQ(mServer.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
        EXPECT_EQ(InitializeCameraAVSMDefaults(mServer), CHIP_NO_ERROR);
        EXPECT_EQ(mServer.Init(), CHIP_NO_ERROR);
    }

    void TearDown() override { app::SetSafeAttributePersistenceProvider(nullptr); }

    std::vector<VideoStreamStruct> mVideoStreams;
    std::vector<AudioStreamStruct> mAudioStreams;
    std::vector<SnapshotStreamStruct> mSnapshotStreams;
    MockCameraAVStreamManagementDelegate mMockDelegate;
    CameraAvStreamManagement::CameraAVStreamManagementCluster mServer;
    ClusterTester mClusterTester;
    app::DefaultSafeAttributePersistenceProvider mPersistenceProvider;
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

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SpeakerVolumeLevel::Id, (uint8_t)100), CHIP_NO_ERROR);
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
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SpeakerVolumeLevel::Id, (uint8_t)(minLevel - 1)), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SpeakerVolumeLevel::Id, (uint8_t)(maxLevel + 1)), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SpeakerVolumeLevel::Id, (uint8_t)1), CHIP_NO_ERROR); // Restore default
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

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::MicrophoneVolumeLevel::Id, (uint8_t)50), CHIP_NO_ERROR);
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
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::MicrophoneVolumeLevel::Id, (uint8_t)(minLevel - 1)), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::MicrophoneVolumeLevel::Id, (uint8_t)(maxLevel + 1)), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::MicrophoneVolumeLevel::Id, (uint8_t)1), CHIP_NO_ERROR); // Restore default
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

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::ImageRotation::Id, (uint16_t)90), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ImageRotation::Id, imageRotation), CHIP_NO_ERROR);
    EXPECT_EQ(imageRotation, 90);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::ImageRotation::Id, (uint16_t)180), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ImageRotation::Id, imageRotation), CHIP_NO_ERROR);
    EXPECT_EQ(imageRotation, 180);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::ImageRotation::Id, (uint16_t)270), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ImageRotation::Id, imageRotation), CHIP_NO_ERROR);
    EXPECT_EQ(imageRotation, 270);

    // Test invalid value
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::ImageRotation::Id, (uint16_t)360), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::ImageRotation::Id, (uint16_t)0), CHIP_NO_ERROR); // Restore default
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

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::StatusLightBrightness::Id, Globals::ThreeLevelAutoEnum::kLow), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::StatusLightBrightness::Id, statusLightBrightness), CHIP_NO_ERROR);
    EXPECT_EQ(statusLightBrightness, Globals::ThreeLevelAutoEnum::kLow);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::StatusLightBrightness::Id, Globals::ThreeLevelAutoEnum::kHigh), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::StatusLightBrightness::Id, statusLightBrightness), CHIP_NO_ERROR);
    EXPECT_EQ(statusLightBrightness, Globals::ThreeLevelAutoEnum::kHigh);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::StatusLightBrightness::Id, Globals::ThreeLevelAutoEnum::kAuto), CHIP_NO_ERROR);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::StatusLightBrightness::Id, statusLightBrightness), CHIP_NO_ERROR);
    EXPECT_EQ(statusLightBrightness, Globals::ThreeLevelAutoEnum::kAuto);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::StatusLightBrightness::Id, Globals::ThreeLevelAutoEnum::kMedium), CHIP_NO_ERROR); // Restore default
}

} // namespace
