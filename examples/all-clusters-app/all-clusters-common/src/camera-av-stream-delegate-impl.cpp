/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/camera-av-stream-management-server/camera-av-stream-management-server.h>
#include <camera-av-stream-delegate-impl.h>
#include <fstream>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>

#define SNAPSHOT_FILE_PATH "./capture_snapshot_test.jpg"
#define SNAPSHOT_FILE_RES_WIDTH (168)
#define SNAPSHOT_FILE_RES_HEIGHT (112)

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Globals;
using namespace chip::app::Clusters::CameraAvStreamManagement;
using chip::Protocols::InteractionModel::Status;

// Global pointer to overall Camera AVStream Manager implementing the Cluster delegate.
std::unique_ptr<CameraAVStreamManager> sCameraAVStreamMgrInstance;

// Global pointer to Camera AVStream Mgmt Server SDK cluster;
std::unique_ptr<CameraAVStreamMgmtServer> sCameraAVStreamMgmtClusterServerInstance;

Protocols::InteractionModel::Status CameraAVStreamManager::VideoStreamAllocate(const VideoStreamStruct & allocateArgs,
                                                                               uint16_t & outStreamID)
{
    outStreamID               = kInvalidStreamID;
    bool foundAvailableStream = false;

    for (VideoStream & stream : videoStreams)
    {
        if (!stream.isAllocated)
        {
            foundAvailableStream = true;

            if (stream.codec == allocateArgs.videoCodec)
            {
                stream.isAllocated = true;
                outStreamID        = stream.id;
                return Status::Success;
            }
        }
    }

    if (!foundAvailableStream)
    {
        return Status::ResourceExhausted;
    }

    return Status::Failure;
}

Protocols::InteractionModel::Status CameraAVStreamManager::VideoStreamModify(const uint16_t streamID,
                                                                             const Optional<bool> waterMarkEnabled,
                                                                             const Optional<bool> osdEnabled)
{
    for (VideoStream & stream : videoStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            ChipLogError(Zcl, "Modified video stream with ID: %d", streamID);
            return Status::Success;
        }
    }

    ChipLogError(Zcl, "Allocated video stream with ID: %d not found", streamID);
    return Status::NotFound;
}

Protocols::InteractionModel::Status CameraAVStreamManager::VideoStreamDeallocate(const uint16_t streamID)
{
    for (VideoStream & stream : videoStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            stream.isAllocated = false;
            break;
        }
    }

    return Status::Success;
}

Protocols::InteractionModel::Status CameraAVStreamManager::AudioStreamAllocate(const AudioStreamStruct & allocateArgs,
                                                                               uint16_t & outStreamID)
{
    outStreamID = kInvalidStreamID;

    bool foundAvailableStream = false;

    for (AudioStream & stream : audioStreams)
    {
        if (!stream.isAllocated)
        {
            foundAvailableStream = true;

            if (stream.codec == allocateArgs.audioCodec)
            {
                stream.isAllocated = true;
                outStreamID        = stream.id;
                return Status::Success;
            }
        }
    }

    if (!foundAvailableStream)
    {
        return Status::ResourceExhausted;
    }

    return Status::Failure;
}

Protocols::InteractionModel::Status CameraAVStreamManager::AudioStreamDeallocate(const uint16_t streamID)
{
    for (AudioStream & stream : audioStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            stream.isAllocated = false;
            break;
        }
    }

    return Status::Success;
}

Protocols::InteractionModel::Status CameraAVStreamManager::SnapshotStreamAllocate(const SnapshotStreamStruct & allocateArgs,
                                                                                  uint16_t & outStreamID)
{
    outStreamID = kInvalidStreamID;

    bool foundAvailableStream = false;

    for (SnapshotStream & stream : snapshotStreams)
    {
        if (!stream.isAllocated)
        {
            foundAvailableStream = true;

            if (stream.codec == allocateArgs.imageCodec)
            {
                stream.isAllocated = true;
                outStreamID        = stream.id;
                return Status::Success;
            }
        }
    }

    if (!foundAvailableStream)
    {
        return Status::ResourceExhausted;
    }

    return Status::Failure;
}

Protocols::InteractionModel::Status CameraAVStreamManager::SnapshotStreamModify(const uint16_t streamID,
                                                                                const Optional<bool> waterMarkEnabled,
                                                                                const Optional<bool> osdEnabled)
{
    for (SnapshotStream & stream : snapshotStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            ChipLogError(Zcl, "Modified snapshot stream with ID: %d", streamID);
            return Status::Success;
        }
    }

    ChipLogError(Zcl, "Allocated snapshot stream with ID: %d not found", streamID);
    return Status::NotFound;
}

Protocols::InteractionModel::Status CameraAVStreamManager::SnapshotStreamDeallocate(const uint16_t streamID)
{
    for (SnapshotStream & stream : snapshotStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            stream.isAllocated = false;
            break;
        }
    }

    return Status::Success;
}

void CameraAVStreamManager::OnVideoStreamAllocated(const VideoStreamStruct & allocatedStream, StreamAllocationAction action)
{
    ChipLogProgress(Zcl, "Video stream has been allocated");
}

void CameraAVStreamManager::OnStreamUsagePrioritiesChanged()
{
    ChipLogProgress(Zcl, "Stream usage priorities changed");
}

void CameraAVStreamManager::OnAttributeChanged(AttributeId attributeId)
{
    ChipLogProgress(Zcl, "Attribute changed for AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
}

Protocols::InteractionModel::Status CameraAVStreamManager::CaptureSnapshot(const Nullable<uint16_t> streamID,
                                                                           const VideoResolutionStruct & resolution,
                                                                           ImageSnapshot & outImageSnapshot)
{
    std::ifstream file(SNAPSHOT_FILE_PATH, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        ChipLogError(Zcl, "Error opening snapshot image file: ");
        return Status::Failure;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Ensure space for image snapshot data in outImageSnapshot
    outImageSnapshot.data.resize(static_cast<size_t>(size));

    if (!file.read(reinterpret_cast<char *>(outImageSnapshot.data.data()), size))
    {
        ChipLogError(Zcl, "Error reading image file: ");
        file.close();
        return Status::Failure;
    }

    file.close();

    outImageSnapshot.imageRes.width  = SNAPSHOT_FILE_RES_WIDTH;
    outImageSnapshot.imageRes.height = SNAPSHOT_FILE_RES_HEIGHT;
    outImageSnapshot.imageCodec      = ImageCodecEnum::kJpeg;

    return Status::Success;
}

CHIP_ERROR
CameraAVStreamManager::LoadAllocatedVideoStreams(std::vector<VideoStreamStruct> & allocatedVideoStreams)
{
    allocatedVideoStreams.clear();

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManager::LoadAllocatedAudioStreams(std::vector<AudioStreamStruct> & allocatedAudioStreams)
{
    allocatedAudioStreams.clear();

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManager::LoadAllocatedSnapshotStreams(std::vector<SnapshotStreamStruct> & allocatedSnapshotStreams)
{
    allocatedSnapshotStreams.clear();

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManager::PersistentAttributesLoadedCallback()
{
    ChipLogDetail(Zcl, "Persistent attributes loaded");

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManager::OnTransportAcquireAudioVideoStreams(uint16_t audioStreamID, uint16_t videoStreamID)
{
    ChipLogDetail(Zcl, "Transport acquired audio/video streams");

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManager::OnTransportReleaseAudioVideoStreams(uint16_t audioStreamID, uint16_t videoStreamID)
{
    ChipLogDetail(Zcl, "Transport released audio/video streams");

    return CHIP_NO_ERROR;
}

void CameraAVStreamManager::InitializeAvailableVideoStreams()
{
    // Example initialization with different codecs
    // id, isAllocated, codec, fps
    videoStreams.push_back({ 0, false, VideoCodecEnum::kH264, 24 });
    videoStreams.push_back({ 1, false, VideoCodecEnum::kH264, 30 });
    videoStreams.push_back({ 2, false, VideoCodecEnum::kHevc, 30 });
    videoStreams.push_back({ 3, false, VideoCodecEnum::kHevc, 60 });
}

void CameraAVStreamManager::InitializeAvailableAudioStreams()
{
    // Example initialization with different codecs
    // id, isAllocated, codec, channelCount
    audioStreams.push_back({ 0, false, AudioCodecEnum::kOpus, 2 });
    audioStreams.push_back({ 1, false, AudioCodecEnum::kOpus, 2 });
    audioStreams.push_back({ 2, false, AudioCodecEnum::kAacLc, 1 });
    audioStreams.push_back({ 3, false, AudioCodecEnum::kAacLc, 2 });
}

void CameraAVStreamManager::InitializeAvailableSnapshotStreams()
{
    // Example initialization with different codecs
    // id, isAllocated, codec, quality
    snapshotStreams.push_back({ 0, false, ImageCodecEnum::kJpeg, 80 });
    snapshotStreams.push_back({ 1, false, ImageCodecEnum::kJpeg, 90 });
    snapshotStreams.push_back({ 2, false, ImageCodecEnum::kJpeg, 90 });
    snapshotStreams.push_back({ 3, false, ImageCodecEnum::kJpeg, 80 });
}

void CameraAVStreamManager::Init()
{
    InitializeAvailableVideoStreams();

    InitializeAvailableAudioStreams();

    InitializeAvailableSnapshotStreams();
}

void emberAfCameraAvStreamManagementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrReturn(endpoint == 1, // this cluster is only enabled for endpoint 1.
                   ChipLogError(Zcl, "AV Stream Mgmt cluster delegate is not implemented for endpoint with id %d.", endpoint));

    VerifyOrReturn(!sCameraAVStreamMgrInstance && !sCameraAVStreamMgmtClusterServerInstance);

    sCameraAVStreamMgrInstance = std::make_unique<CameraAVStreamManager>();
    sCameraAVStreamMgrInstance->Init();

    BitFlags<Feature> features;
    features.Set(Feature::kSnapshot);
    features.Set(Feature::kVideo);
    features.Set(Feature::kNightVision);
    features.Set(Feature::kImageControl);
    features.Set(Feature::kAudio);
    features.Set(Feature::kPrivacy);
    features.Set(Feature::kSpeaker);
    features.Set(Feature::kLocalStorage);
    features.Set(Feature::kWatermark);
    features.Set(Feature::kOnScreenDisplay);
    features.Set(Feature::kHighDynamicRange);

    // Pure optional attributes that aren't covered by a feature flag, or are attested by the server given feature flag settings
    BitFlags<OptionalAttribute> optionalAttrs;
    optionalAttrs.Set(OptionalAttribute::kHardPrivacyModeOn);
    optionalAttrs.Set(OptionalAttribute::kNightVisionIllum);
    optionalAttrs.Set(OptionalAttribute::kMicrophoneAGCEnabled);
    optionalAttrs.Set(OptionalAttribute::kStatusLightEnabled);
    optionalAttrs.Set(OptionalAttribute::kStatusLightBrightness);
    optionalAttrs.Set(OptionalAttribute::kImageFlipVertical);
    optionalAttrs.Set(OptionalAttribute::kImageFlipHorizontal);
    optionalAttrs.Set(OptionalAttribute::kImageRotation);

    uint32_t maxConcurrentVideoEncoders  = 1;
    uint32_t maxEncodedPixelRate         = 10000;
    VideoSensorParamsStruct sensorParams = { 4608, 2592, 120, Optional<uint16_t>(30) }; // Typical numbers for Pi camera.
    bool nightVisionUsesInfrared         = false;
    VideoResolutionStruct minViewport    = { 854, 480 }; // Assuming 480p resolution.
    std::vector<RateDistortionTradeOffStruct> rateDistortionTradeOffPoints = {};
    uint32_t maxContentBufferSize                                          = 1024;
    AudioCapabilitiesStruct micCapabilities{};
    AudioCapabilitiesStruct spkrCapabilities{};
    TwoWayTalkSupportTypeEnum twowayTalkSupport                  = TwoWayTalkSupportTypeEnum::kNotSupported;
    std::vector<SnapshotCapabilitiesStruct> snapshotCapabilities = {};
    uint32_t maxNetworkBandwidth                                 = 64;
    std::vector<StreamUsageEnum> supportedStreamUsages           = { StreamUsageEnum::kLiveView, StreamUsageEnum::kRecording };
    std::vector<StreamUsageEnum> streamUsagePriorities           = { StreamUsageEnum::kLiveView, StreamUsageEnum::kRecording };

    sCameraAVStreamMgmtClusterServerInstance = std::make_unique<CameraAVStreamMgmtServer>(
        *sCameraAVStreamMgrInstance.get(), endpoint, features, optionalAttrs, maxConcurrentVideoEncoders, maxEncodedPixelRate,
        sensorParams, nightVisionUsesInfrared, minViewport, rateDistortionTradeOffPoints, maxContentBufferSize, micCapabilities,
        spkrCapabilities, twowayTalkSupport, snapshotCapabilities, maxNetworkBandwidth, supportedStreamUsages,
        streamUsagePriorities);
    sCameraAVStreamMgmtClusterServerInstance->Init();
}

void emberAfCameraAvStreamManagementClusterShutdownCallback(EndpointId endpoint)
{
    sCameraAVStreamMgmtClusterServerInstance = nullptr;
    sCameraAVStreamMgrInstance               = nullptr;
}
