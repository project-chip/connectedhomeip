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

#include <app/CommandHandler.h>
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
        mAllocatedAudioStreams(audioStreams),
        mAllocatedSnapshotStreams(snapshotStreams)
    {}

    Protocols::InteractionModel::Status VideoStreamAllocate(const VideoStreamStruct & allocateArgs, uint16_t & outStreamID) override
    {
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
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status AudioStreamDeallocate(const uint16_t streamID) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status SnapshotStreamAllocate(const SnapshotStreamAllocateArgs & allocateArgs,
                                                               uint16_t & outStreamID) override
    {
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

    std::vector<VideoStreamStruct> mVideoStreams;
    std::vector<AudioStreamStruct> mAudioStreams;
    std::vector<SnapshotStreamStruct> mSnapshotStreams;
};

TEST_F(TestCameraAVStreamManagementCluster, TestAttributes)
{
    MockCameraAVStreamManagementDelegate mockDelegate(&mVideoStreams, &mAudioStreams, &mSnapshotStreams);
    CameraAvStreamManagement::CameraAVStreamManagementCluster server(
        mockDelegate, kTestEndpointId,
        chip::BitFlags<CameraAvStreamManagement::Feature>(
            CameraAvStreamManagement::Feature::kVideo, CameraAvStreamManagement::Feature::kAudio,
            CameraAvStreamManagement::Feature::kSnapshot, CameraAvStreamManagement::Feature::kSpeaker,
            CameraAvStreamManagement::Feature::kImageControl, CameraAvStreamManagement::Feature::kPrivacy),
        chip::BitFlags<CameraAvStreamManagement::OptionalAttribute>(
            CameraAvStreamManagement::OptionalAttribute::kHardPrivacyModeOn,
            CameraAvStreamManagement::OptionalAttribute::kMicrophoneAGCEnabled,
            CameraAvStreamManagement::OptionalAttribute::kImageRotation,
            CameraAvStreamManagement::OptionalAttribute::kImageFlipHorizontal,
            CameraAvStreamManagement::OptionalAttribute::kImageFlipVertical,
            CameraAvStreamManagement::OptionalAttribute::kStatusLightEnabled,
            CameraAvStreamManagement::OptionalAttribute::kStatusLightBrightness),
        0, 0, {}, false, {}, {}, 0, {}, {}, TwoWayTalkSupportTypeEnum::kFullDuplex, {}, 0, {}, {});

    ASSERT_TRUE(
        Testing::IsAttributesListEqualTo(server,
                                         {
                                             CameraAvStreamManagement::Attributes::MaxConcurrentEncoders::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MaxEncodedPixelRate::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::VideoSensorParams::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MinViewportResolution::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::RateDistortionTradeOffPoints::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MaxContentBufferSize::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MicrophoneCapabilities::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::SpeakerCapabilities::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::TwoWayTalkSupport::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::SnapshotCapabilities::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::MaxNetworkBandwidth::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::CurrentFrameRate::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::SupportedStreamUsages::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::AllocatedVideoStreams::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::AllocatedAudioStreams::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::AllocatedSnapshotStreams::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::StreamUsagePriorities::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::SoftRecordingPrivacyModeEnabled::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::SoftLivestreamPrivacyModeEnabled::kMetadataEntry,
                                             CameraAvStreamManagement::Attributes::HardPrivacyModeOn::kMetadataEntry,
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

TEST_F(TestCameraAVStreamManagementCluster, TestCommands)
{
    MockCameraAVStreamManagementDelegate mockDelegate(&mVideoStreams, &mAudioStreams, &mSnapshotStreams);
    CameraAvStreamManagement::CameraAVStreamManagementCluster server(
        mockDelegate, kTestEndpointId,
        chip::BitFlags<CameraAvStreamManagement::Feature>(CameraAvStreamManagement::Feature::kVideo,
                                                          CameraAvStreamManagement::Feature::kAudio,
                                                          CameraAvStreamManagement::Feature::kSnapshot),
        chip::BitFlags<CameraAvStreamManagement::OptionalAttribute>(
            CameraAvStreamManagement::OptionalAttribute::kHardPrivacyModeOn,
            CameraAvStreamManagement::OptionalAttribute::kNightVisionIllum,
            CameraAvStreamManagement::OptionalAttribute::kMicrophoneAGCEnabled,
            CameraAvStreamManagement::OptionalAttribute::kImageRotation,
            CameraAvStreamManagement::OptionalAttribute::kImageFlipHorizontal,
            CameraAvStreamManagement::OptionalAttribute::kImageFlipVertical,
            CameraAvStreamManagement::OptionalAttribute::kStatusLightEnabled,
            CameraAvStreamManagement::OptionalAttribute::kStatusLightBrightness),
        0, 0, {}, false, {}, {}, 0, {}, {}, TwoWayTalkSupportTypeEnum::kFullDuplex, {}, 0, {}, {});

    ASSERT_TRUE(
        Testing::IsAcceptedCommandsListEqualTo(server,
                                               {
                                                   CameraAvStreamManagement::Commands::VideoStreamAllocate::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::VideoStreamDeallocate::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::AudioStreamAllocate::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::AudioStreamDeallocate::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::SnapshotStreamAllocate::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::SnapshotStreamDeallocate::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::SetStreamPriorities::kMetadataEntry,
                                                   CameraAvStreamManagement::Commands::CaptureSnapshot::kMetadataEntry,
                                               }));
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadClusterRevisionAttribute)
{
    MockCameraAVStreamManagementDelegate mockDelegate(&mVideoStreams, &mAudioStreams, &mSnapshotStreams);
    CameraAvStreamManagement::CameraAVStreamManagementCluster server(
        mockDelegate, kTestEndpointId,
        chip::BitFlags<CameraAvStreamManagement::Feature>(CameraAvStreamManagement::Feature::kVideo,
                                                          CameraAvStreamManagement::Feature::kAudio,
                                                          CameraAvStreamManagement::Feature::kSnapshot),
        chip::BitFlags<CameraAvStreamManagement::OptionalAttribute>(
            CameraAvStreamManagement::OptionalAttribute::kHardPrivacyModeOn,
            CameraAvStreamManagement::OptionalAttribute::kNightVisionIllum,
            CameraAvStreamManagement::OptionalAttribute::kMicrophoneAGCEnabled,
            CameraAvStreamManagement::OptionalAttribute::kImageRotation,
            CameraAvStreamManagement::OptionalAttribute::kImageFlipHorizontal,
            CameraAvStreamManagement::OptionalAttribute::kImageFlipVertical,
            CameraAvStreamManagement::OptionalAttribute::kStatusLightEnabled,
            CameraAvStreamManagement::OptionalAttribute::kStatusLightBrightness),
        0, 0, {}, false, {}, {}, 0, {}, {}, TwoWayTalkSupportTypeEnum::kFullDuplex, {}, 0, {}, {});

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
    auto status = server.ReadAttribute(request, encoder);
    EXPECT_TRUE(status.IsSuccess());
}

TEST_F(TestCameraAVStreamManagementCluster, TestReadMaxConcurrentEncoders)
{
    uint8_t selectedChime = 1;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MaxConcurrentEncoders::Id, selectedChime), CHIP_NO_ERROR);
    EXPECT_EQ(selectedChime, 0);
}

TEST_F(TestCameraAVStreamManagementCluster, TestSnapshotStreamAllocate)
{
    MockCameraAVStreamManagementDelegate mockDelegate(&mVideoStreams, &mAudioStreams, &mSnapshotStreams);
    CameraAvStreamManagement::CameraAVStreamManagementCluster server(
        mockDelegate, kTestEndpointId,
        chip::BitFlags<CameraAvStreamManagement::Feature>(CameraAvStreamManagement::Feature::kSnapshot),
        chip::BitFlags<CameraAvStreamManagement::OptionalAttribute>(),
        0, 0, {}, false, {}, {}, 0, {}, {}, TwoWayTalkSupportTypeEnum::kFullDuplex, {}, 0, {}, {});

    ClusterTester clusterTester(server);

    Commands::SnapshotStreamAllocate::Type request;
    //chip::app::Clusters::CameraAvStreamManagement::Commands::SnapshotStreamAllocate::Type request;
    auto result = clusterTester.Invoke(Commands::SnapshotStreamAllocate::Id, request);
     EXPECT_TRUE(result.IsSuccess());
     EXPECT_TRUE(result.response.has_value());
    // TODO: Need a way to invoke the command and check the response
    // This requires a command invocation framework within the test.
    // For now, this test just sets up the request.
}

TEST_F(TestCameraAVStreamManagementCluster, TestSnapshotStreamModify)
{
    MockCameraAVStreamManagementDelegate mockDelegate(&mVideoStreams, &mAudioStreams, &mSnapshotStreams);
    CameraAvStreamManagement::CameraAVStreamManagementCluster server(
        mockDelegate, kTestEndpointId,
        chip::BitFlags<CameraAvStreamManagement::Feature>(CameraAvStreamManagement::Feature::kSnapshot),
        chip::BitFlags<CameraAvStreamManagement::OptionalAttribute>(),
        0, 0, {}, false, {}, {}, 0, {}, {}, TwoWayTalkSupportTypeEnum::kFullDuplex, {}, 0, {}, {});

    ClusterTester clusterTester(server);
    Commands::SnapshotStreamModify::Type request;
    request.snapshotStreamID = 1; // Assuming a stream ID
    request.watermarkEnabled.SetValue(true);
    request.OSDEnabled.SetValue(true);

    auto result = clusterTester.Invoke(Commands::SnapshotStreamModify::Id, request);
    EXPECT_TRUE(result.IsSuccess());
}

TEST_F(TestCameraAVStreamManagementCluster, TestSnapshotStreamDeallocate)
{
    MockCameraAVStreamManagementDelegate mockDelegate(&mVideoStreams, &mAudioStreams, &mSnapshotStreams);
    CameraAvStreamManagement::CameraAVStreamManagementCluster server(
        mockDelegate, kTestEndpointId,
        chip::BitFlags<CameraAvStreamManagement::Feature>(CameraAvStreamManagement::Feature::kSnapshot),
        chip::BitFlags<CameraAvStreamManagement::OptionalAttribute>(),
        0, 0, {}, false, {}, {}, 0, {}, {}, TwoWayTalkSupportTypeEnum::kFullDuplex, {}, 0, {}, {});

    ClusterTester clusterTester(server);
    chip::app::Clusters::CameraAvStreamManagement::Commands::SnapshotStreamDeallocate::Type request;
    request.snapshotStreamID = 1; // Assuming a stream ID

    auto result = clusterTester.Invoke(Commands::SnapshotStreamDeallocate::Id, request);
    EXPECT_TRUE(result.IsSuccess());
}

} // namespace
