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
#include <camera-av-stream-manager.h>
#include <fstream>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>
#include <set>

#define SNAPSHOT_FILE_PATH "./capture_snapshot.jpg"
#define SNAPSHOT_FILE_RES_WIDTH (168)
#define SNAPSHOT_FILE_RES_HEIGHT (112)

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvStreamManagement;
using namespace chip::app::Clusters::CameraAvStreamManagement::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

// Constants
constexpr uint16_t kInvalidStreamID = 65500;

} // namespace

void CameraAVStreamManager::SetCameraDeviceHAL(CameraDeviceInterface * aCameraDeviceHAL)
{
    mCameraDeviceHAL = aCameraDeviceHAL;
}

CHIP_ERROR CameraAVStreamManager::ValidateStreamUsage(StreamUsageEnum streamUsage,
                                                      Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                                      Optional<DataModel::Nullable<uint16_t>> & audioStreamId)
{
    // The server ensures that at least one stream Id has a value, and that there are streams allocated
    // If a stream id(s) are provided, it's sufficient to have verified that the provide usage is supported by the camera.
    // If they're Null, look for a stream ID that matches the usage. A match does not need to be exact.
    bool exactlyMatchedVideoStream = false;
    bool looselyMatchedVideoStream = false;
    uint16_t looseVideoStreamID;
    bool exactlyMatchedAudioStream = false;
    bool looselyMatchedAudioStream = false;
    uint16_t looseAudioStreamID;

    // Is the requested stream usage supported by the camera?
    auto myStreamUsages = GetCameraAVStreamManagementCluster()->GetSupportedStreamUsages();
    auto it             = std::find(myStreamUsages.begin(), myStreamUsages.end(), streamUsage);
    if (it == myStreamUsages.end())
    {
        ChipLogError(Camera, "Requested stream usage not found in supported stream usages");
        return CHIP_ERROR_NOT_FOUND;
    }

    if (videoStreamId.HasValue())
    {
        const std::vector<VideoStreamStruct> & allocatedVideoStreams =
            GetCameraAVStreamManagementCluster()->GetAllocatedVideoStreams();

        // If no Video ID is provided, match to an allocated ID. Exact is preferred if found.  We know the stream requested is in
        // supported streams.
        if (videoStreamId.Value().IsNull())
        {
            for (const auto & stream : allocatedVideoStreams)
            {
                if (stream.streamUsage == streamUsage)
                {
                    videoStreamId.Emplace(stream.videoStreamID);
                    exactlyMatchedVideoStream = true;
                    break;
                }

                looselyMatchedVideoStream = true;
                looseVideoStreamID        = stream.videoStreamID;
            }
        }
        else
        {
            // We've been provided with a stream ID, and we know the stream usage is supported by the camera, classify as an exact
            // match
            exactlyMatchedVideoStream = true;
        }
    }

    if (audioStreamId.HasValue())
    {
        const std::vector<AudioStreamStruct> & allocatedAudioStreams =
            GetCameraAVStreamManagementCluster()->GetAllocatedAudioStreams();

        // If no Audio ID is provided, match to an allocated ID. Exact is preferred if found.  We know the stream requested is in
        // supported streams.
        if (audioStreamId.Value().IsNull())
        {
            for (const auto & stream : allocatedAudioStreams)
            {
                if (stream.streamUsage == streamUsage)
                {
                    audioStreamId.Emplace(stream.audioStreamID);
                    exactlyMatchedAudioStream = true;
                    break;
                }

                looselyMatchedAudioStream = true;
                looseAudioStreamID        = stream.audioStreamID;
            }
        }
        else
        {
            // We've been provided with a stream ID, and we know the stream usage is supported by the camera, classify as an exact
            // match
            exactlyMatchedAudioStream = true;
        }
    }

    // If we have a loose match and no exact match, update the provided stream IDs with the loose match values
    //
    if (looselyMatchedAudioStream && !exactlyMatchedAudioStream)
    {
        audioStreamId.Emplace(looseAudioStreamID);
    }

    if (looselyMatchedVideoStream && !exactlyMatchedVideoStream)
    {
        videoStreamId.Emplace(looseVideoStreamID);
    }

    return CHIP_NO_ERROR;
}

const std::vector<chip::app::Clusters::CameraAvStreamManagement::VideoStreamStruct> &
CameraAVStreamManager::GetAllocatedVideoStreams() const
{
    return GetCameraAVStreamManagementCluster()->GetAllocatedVideoStreams();
}

const std::vector<chip::app::Clusters::CameraAvStreamManagement::AudioStreamStruct> &
CameraAVStreamManager::GetAllocatedAudioStreams() const
{
    return GetCameraAVStreamManagementCluster()->GetAllocatedAudioStreams();
}

void CameraAVStreamManager::GetBandwidthForStreams(const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                                   const Optional<DataModel::Nullable<uint16_t>> & audioStreamId,
                                                   uint32_t & outBandwidthbps)
{

    outBandwidthbps = 0;
    if (videoStreamId.HasValue() && !videoStreamId.Value().IsNull())
    {
        uint16_t vStreamId           = videoStreamId.Value().Value();
        auto & allocatedVideoStreams = GetCameraAVStreamManagementCluster()->GetAllocatedVideoStreams();
        for (const chip::app::Clusters::CameraAvStreamManagement::Structs::VideoStreamStruct::Type & stream : allocatedVideoStreams)
        {
            if (stream.videoStreamID == vStreamId)
            {
                outBandwidthbps += stream.maxBitRate;
                ChipLogProgress(Camera, "GetBandwidthForStreams: VideoStream %u maxBitRate: %u bps", vStreamId, stream.maxBitRate);
                break;
            }
        }
    }
    if (audioStreamId.HasValue() && !audioStreamId.Value().IsNull())
    {
        uint16_t aStreamId           = audioStreamId.Value().Value();
        auto & allocatedAudioStreams = GetCameraAVStreamManagementCluster()->GetAllocatedAudioStreams();
        for (const chip::app::Clusters::CameraAvStreamManagement::Structs::AudioStreamStruct::Type & stream : allocatedAudioStreams)
        {
            if (stream.audioStreamID == aStreamId)
            {
                outBandwidthbps += stream.bitRate;
                ChipLogProgress(Camera, "GetBandwidthForStreams: AudioStream %u bitRate: %u bps", aStreamId, stream.bitRate);
                break;
            }
        }
    }
    return;
}

CHIP_ERROR CameraAVStreamManager::ValidateVideoStreamID(uint16_t videoStreamId)
{
    const std::vector<VideoStreamStruct> & allocatedVideoStreams = GetCameraAVStreamManagementCluster()->GetAllocatedVideoStreams();

    // Check if the videoStreamId exists in allocated streams
    for (const auto & stream : allocatedVideoStreams)
    {
        if (stream.videoStreamID == videoStreamId)
        {
            ChipLogProgress(Camera, "Video stream ID %u is valid and allocated", videoStreamId);
            return CHIP_NO_ERROR;
        }
    }

    ChipLogError(Camera, "Video stream ID %u not found in allocated video streams", videoStreamId);
    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR CameraAVStreamManager::ValidateAudioStreamID(uint16_t audioStreamId)
{
    const std::vector<AudioStreamStruct> & allocatedAudioStreams = GetCameraAVStreamManagementCluster()->GetAllocatedAudioStreams();

    // Check if the audioStreamId exists in allocated streams
    for (const auto & stream : allocatedAudioStreams)
    {
        if (stream.audioStreamID == audioStreamId)
        {
            ChipLogProgress(Camera, "Audio stream ID %u is valid and allocated", audioStreamId);
            return CHIP_NO_ERROR;
        }
    }

    ChipLogError(Camera, "Audio stream ID %u not found in allocated audio streams", audioStreamId);
    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR CameraAVStreamManager::IsHardPrivacyModeActive(bool & isActive)
{
    // Check privacy mode attributes
    isActive = GetCameraAVStreamManagementCluster()->GetHardPrivacyModeOn();
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamManager::IsSoftRecordingPrivacyModeActive(bool & isActive)
{
    // Check privacy mode attributes
    isActive = GetCameraAVStreamManagementCluster()->GetSoftRecordingPrivacyModeEnabled();
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamManager::IsSoftLivestreamPrivacyModeActive(bool & isActive)
{
    // Check privacy mode attributes
    isActive = GetCameraAVStreamManagementCluster()->GetSoftLivestreamPrivacyModeEnabled();
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAVStreamManager::SetHardPrivacyModeOn(bool hardPrivacyMode)
{
    return GetCameraAVStreamManagementCluster()->SetHardPrivacyModeOn(hardPrivacyMode);
}

bool CameraAVStreamManager::HasAllocatedVideoStreams()
{
    const std::vector<VideoStreamStruct> & allocatedVideoStreams = GetCameraAVStreamManagementCluster()->GetAllocatedVideoStreams();
    return !allocatedVideoStreams.empty();
}

bool CameraAVStreamManager::HasAllocatedAudioStreams()
{
    const std::vector<AudioStreamStruct> & allocatedAudioStreams = GetCameraAVStreamManagementCluster()->GetAllocatedAudioStreams();
    return !allocatedAudioStreams.empty();
}

Protocols::InteractionModel::Status CameraAVStreamManager::VideoStreamAllocate(const VideoStreamStruct & allocateArgs,
                                                                               uint16_t & outStreamID)
{
    outStreamID                                 = kInvalidStreamID;
    bool isRequestSupportedByAnyAvailableStream = false;

    // Check if allocation request can be supported
    for (const auto & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams())
    {
        if (stream.IsCompatible(allocateArgs))
        {
            isRequestSupportedByAnyAvailableStream = true;
            break;
        }
    }

    if (!isRequestSupportedByAnyAvailableStream)
    {
        return Status::DynamicConstraintError;
    }

    // Try to reuse an allocated stream
    std::optional<uint16_t> reusableStreamId = GetCameraAVStreamManagementCluster()->GetReusableVideoStreamId(allocateArgs);

    if (reusableStreamId.has_value())
    {
        // Found a stream that can be reused
        outStreamID = reusableStreamId.value();
        ChipLogProgress(Camera, "Matching pre-allocated stream with ID: %d exists", outStreamID);
        return Status::Success;
    }

    // Try to find an unused compatible available stream
    for (auto & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams())
    {
        if (!stream.isAllocated && stream.IsCompatible(allocateArgs))
        {
            uint32_t candidateEncodedPixelRate =
                allocateArgs.maxFrameRate * allocateArgs.maxResolution.height * allocateArgs.maxResolution.width;
            bool encoderRequired = true;
            if (!GetCameraAVStreamManagementCluster()->IsResourceAvailableForStreamAllocation(candidateEncodedPixelRate,
                                                                                              encoderRequired))
            {
                return Status::ResourceExhausted;
            }
            stream.isAllocated = true;
            outStreamID        = stream.videoStreamParams.videoStreamID;

            // Set the default viewport on the newly allocated stream
            mCameraDeviceHAL->GetCameraHALInterface().SetViewport(stream, mCameraDeviceHAL->GetCameraHALInterface().GetViewport());

            // Inform DPTZ that there's an allocated stream
            mCameraDeviceHAL->GetCameraAVSettingsUserLevelMgmtDelegate().VideoStreamAllocated(outStreamID);

            // Set the current frame rate attribute from HAL
            TEMPORARY_RETURN_IGNORED GetCameraAVStreamManagementCluster()->SetCurrentFrameRate(
                mCameraDeviceHAL->GetCameraHALInterface().GetCurrentFrameRate());

            return Status::Success;
        }
    }

    // No compatible stream available for use.
    return Status::ResourceExhausted;
}

void CameraAVStreamManager::OnVideoStreamAllocated(const VideoStreamStruct & allocatedStream, StreamAllocationAction action)
{
    switch (action)
    {
    case StreamAllocationAction::kNewAllocation:
        ChipLogProgress(Camera, "Starting new video stream with ID: %u", allocatedStream.videoStreamID);
        mCameraDeviceHAL->GetCameraHALInterface().StartVideoStream(allocatedStream);

        // Set the current frame rate attribute from HAL once stream has started
        TEMPORARY_RETURN_IGNORED GetCameraAVStreamManagementCluster()->SetCurrentFrameRate(
            mCameraDeviceHAL->GetCameraHALInterface().GetCurrentFrameRate());
        break;

    case StreamAllocationAction::kModification:
        // Find the stream and restart it with new parameters
        for (VideoStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams())
        {
            if (stream.videoStreamParams.videoStreamID == allocatedStream.videoStreamID && stream.isAllocated)
            {
                // For modifications, we always stop and restart the stream to ensure new parameters are applied
                ChipLogProgress(Camera, "Restarting video stream with ID: %u due to modifications", allocatedStream.videoStreamID);
                mCameraDeviceHAL->GetCameraHALInterface().StopVideoStream(allocatedStream.videoStreamID);
                mCameraDeviceHAL->GetCameraHALInterface().StartVideoStream(allocatedStream);
                break;
            }
        }
        break;

    case StreamAllocationAction::kReuse:
        ChipLogProgress(Camera, "Reusing existing video stream with ID: %u without changes", allocatedStream.videoStreamID);
        break;
    }
}

Protocols::InteractionModel::Status CameraAVStreamManager::VideoStreamModify(const uint16_t streamID,
                                                                             const chip::Optional<bool> waterMarkEnabled,
                                                                             const chip::Optional<bool> osdEnabled)
{
    for (VideoStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams())
    {
        if (stream.videoStreamParams.videoStreamID == streamID && stream.isAllocated)
        {
            // TODO: Link with HAL APIs to return error
            if (waterMarkEnabled.HasValue())
            {
                stream.videoStreamParams.watermarkEnabled = waterMarkEnabled;
            }
            if (osdEnabled.HasValue())
            {
                stream.videoStreamParams.OSDEnabled = osdEnabled;
            }
            ChipLogError(Camera, "Modified video stream with ID: %d", streamID);
            return Status::Success;
        }
    }

    ChipLogError(Camera, "Allocated video stream with ID: %d not found", streamID);

    return Status::NotFound;
}

Protocols::InteractionModel::Status CameraAVStreamManager::VideoStreamDeallocate(const uint16_t streamID)
{
    for (VideoStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams())
    {
        if (stream.videoStreamParams.videoStreamID == streamID && stream.isAllocated)
        {
            // Stop the video stream
            mCameraDeviceHAL->GetCameraHALInterface().StopVideoStream(streamID);

            stream.isAllocated = false;

            mCameraDeviceHAL->GetCameraAVSettingsUserLevelMgmtDelegate().VideoStreamDeallocated(streamID);

            return Status::Success;
        }
    }

    ChipLogError(Camera, "Allocated video stream with ID: %d not found internally", streamID);

    return Status::NotFound;
}

Protocols::InteractionModel::Status CameraAVStreamManager::AudioStreamAllocate(const AudioStreamStruct & allocateArgs,
                                                                               uint16_t & outStreamID)
{
    outStreamID = kInvalidStreamID;

    for (AudioStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableAudioStreams())
    {
        if (stream.IsCompatible(allocateArgs))
        {
            outStreamID = stream.audioStreamParams.audioStreamID;
            if (!stream.isAllocated)
            {
                stream.isAllocated = true;

                // Start the audio stream from HAL for serving.
                mCameraDeviceHAL->GetCameraHALInterface().StartAudioStream(outStreamID);

                return Status::Success;
            }
            else
            {
                ChipLogProgress(Camera, "Matching pre-allocated stream with ID: %d exists", outStreamID);
            }
            return Status::Success;
        }
    }

    return Status::DynamicConstraintError;
}

Protocols::InteractionModel::Status CameraAVStreamManager::AudioStreamDeallocate(const uint16_t streamID)
{
    for (AudioStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableAudioStreams())
    {
        if (stream.audioStreamParams.audioStreamID == streamID && stream.isAllocated)
        {
            // Stop the audio stream
            mCameraDeviceHAL->GetCameraHALInterface().StopAudioStream(streamID);

            stream.isAllocated = false;

            return Status::Success;
        }
    }

    ChipLogError(Camera, "Allocated audio stream with ID: %d not found internally", streamID);

    return Status::NotFound;
}

Protocols::InteractionModel::Status CameraAVStreamManager::SnapshotStreamAllocate(const SnapshotStreamAllocateArgs & allocateArgs,
                                                                                  uint16_t & outStreamID)
{
    outStreamID                                 = kInvalidStreamID;
    bool isRequestSupportedByAnyAvailableStream = false;

    // Check if allocation request can be supported
    for (const auto & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableSnapshotStreams())
    {
        if (stream.IsCompatible(allocateArgs))
        {
            isRequestSupportedByAnyAvailableStream = true;
            break;
        }
    }

    if (!isRequestSupportedByAnyAvailableStream)
    {
        return Status::DynamicConstraintError;
    }

    // Try to reuse an allocated stream.
    std::optional<uint16_t> reusableStreamId = GetCameraAVStreamManagementCluster()->GetReusableSnapshotStreamId(allocateArgs);

    if (reusableStreamId.has_value())
    {
        // Found a stream that can be reused
        outStreamID = reusableStreamId.value();
        ChipLogProgress(Camera, "Matching pre-allocated stream with ID: %d exists", outStreamID);
        return Status::Success;
    }

    uint32_t candidateEncodedPixelRate = 0;
    bool encoderRequired               = false;
    if (allocateArgs.encodedPixels)
    {
        candidateEncodedPixelRate +=
            allocateArgs.maxFrameRate * allocateArgs.maxResolution.height * allocateArgs.maxResolution.width;
        if (allocateArgs.hardwareEncoder)
        {
            encoderRequired = true;
        }
    }

    if (!GetCameraAVStreamManagementCluster()->IsResourceAvailableForStreamAllocation(candidateEncodedPixelRate, encoderRequired))
    {
        return Status::ResourceExhausted;
    }

    // If no pre-allocated stream matches, try allocating a new one.
    if (mCameraDeviceHAL->GetCameraHALInterface().AllocateSnapshotStream(allocateArgs, outStreamID) == CameraError::SUCCESS)
    {
        mCameraDeviceHAL->GetCameraHALInterface().StartSnapshotStream(outStreamID);
        return Status::Success;
    }

    // Try to find an unused compatible available stream
    for (auto & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableSnapshotStreams())
    {
        if (!stream.isAllocated && stream.IsCompatible(allocateArgs))
        {
            stream.isAllocated = true;
            outStreamID        = stream.snapshotStreamParams.snapshotStreamID;

            // Set the optional Watermark and OSD values that may have been provided.  This is the initial
            // setting of these values, they may be subsequently modified. If the values have no value that
            // is ok, the allocated stream will store as such and ignore.
            stream.snapshotStreamParams.watermarkEnabled = allocateArgs.watermarkEnabled;
            stream.snapshotStreamParams.OSDEnabled       = allocateArgs.OSDEnabled;

            // Start the snapshot stream for serving.
            mCameraDeviceHAL->GetCameraHALInterface().StartSnapshotStream(outStreamID);

            return Status::Success;
        }
    }

    // No compatible stream available for use.
    return Status::ResourceExhausted;
}

Protocols::InteractionModel::Status CameraAVStreamManager::SnapshotStreamModify(const uint16_t streamID,
                                                                                const chip::Optional<bool> waterMarkEnabled,
                                                                                const chip::Optional<bool> osdEnabled)
{
    for (SnapshotStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableSnapshotStreams())
    {
        if (stream.snapshotStreamParams.snapshotStreamID == streamID && stream.isAllocated)
        {
            // TODO: Link with HAL APIs to return error
            if (waterMarkEnabled.HasValue())
            {
                stream.snapshotStreamParams.watermarkEnabled = waterMarkEnabled;
            }
            if (osdEnabled.HasValue())
            {
                stream.snapshotStreamParams.OSDEnabled = osdEnabled;
            }
            ChipLogError(Camera, "Modified snapshot stream with ID: %d", streamID);
            return Status::Success;
        }
    }

    ChipLogError(Camera, "Allocated snapshot stream with ID: %d not found", streamID);

    return Status::NotFound;
}

Protocols::InteractionModel::Status CameraAVStreamManager::SnapshotStreamDeallocate(const uint16_t streamID)
{
    for (SnapshotStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableSnapshotStreams())
    {
        if (stream.snapshotStreamParams.snapshotStreamID == streamID && stream.isAllocated)
        {
            if (stream.snapshotStreamParams.referenceCount > 0)
            {
                ChipLogError(Camera, "Snapshot stream with ID: %d still in use", streamID);
                return Status::InvalidInState;
            }
            // Stop the snapshot stream for serving.
            mCameraDeviceHAL->GetCameraHALInterface().StopSnapshotStream(streamID);

            stream.isAllocated = false;

            return Status::Success;
        }
    }

    ChipLogError(Camera, "Allocated snapshot stream with ID: %d not found", streamID);

    return Status::NotFound;
}

void CameraAVStreamManager::OnStreamUsagePrioritiesChanged()
{
    ChipLogProgress(Camera, "Stream usage priorities changed");
    mCameraDeviceHAL->GetCameraHALInterface().SetStreamUsagePriorities(
        GetCameraAVStreamManagementCluster()->GetStreamUsagePriorities());
}

void CameraAVStreamManager::OnAttributeChanged(AttributeId attributeId)
{
    ChipLogProgress(Camera, "Attribute changed for AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));

    switch (attributeId)
    {
    case HDRModeEnabled::Id: {

        mCameraDeviceHAL->GetCameraHALInterface().SetHDRMode(GetCameraAVStreamManagementCluster()->GetHDRModeEnabled());
        break;
    }
    case SoftRecordingPrivacyModeEnabled::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetSoftRecordingPrivacyModeEnabled(
            GetCameraAVStreamManagementCluster()->GetSoftRecordingPrivacyModeEnabled());
        break;
    }
    case SoftLivestreamPrivacyModeEnabled::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetSoftLivestreamPrivacyModeEnabled(
            GetCameraAVStreamManagementCluster()->GetSoftLivestreamPrivacyModeEnabled());
        break;
    }
    case HardPrivacyModeOn::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetHardPrivacyMode(GetCameraAVStreamManagementCluster()->GetHardPrivacyModeOn());
        break;
    }
    case NightVision::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetNightVision(GetCameraAVStreamManagementCluster()->GetNightVision());
        break;
    }
    case NightVisionIllum::Id: {
        break;
    }
    case Viewport::Id: {
        // Update the device default
        mCameraDeviceHAL->GetCameraHALInterface().SetViewport(GetCameraAVStreamManagementCluster()->GetViewport());

        // Update the per stream viewports on the camera
        for (VideoStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams())
        {
            mCameraDeviceHAL->GetCameraHALInterface().SetViewport(stream, GetCameraAVStreamManagementCluster()->GetViewport());
        }

        // Inform DPTZ (the server) that the camera default viewport has changed
        mCameraDeviceHAL->GetCameraAVSettingsUserLevelMgmtDelegate().DefaultViewportUpdated(
            GetCameraAVStreamManagementCluster()->GetViewport());
        break;
    }
    case SpeakerMuted::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetSpeakerMuted(GetCameraAVStreamManagementCluster()->GetSpeakerMuted());
        break;
    }
    case SpeakerVolumeLevel::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetSpeakerVolume(GetCameraAVStreamManagementCluster()->GetSpeakerVolumeLevel());
        break;
    }
    case MicrophoneMuted::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetMicrophoneMuted(GetCameraAVStreamManagementCluster()->GetMicrophoneMuted());
        break;
    }
    case MicrophoneVolumeLevel::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetMicrophoneVolume(
            GetCameraAVStreamManagementCluster()->GetMicrophoneVolumeLevel());
        break;
    }
    case LocalVideoRecordingEnabled::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetLocalVideoRecordingEnabled(
            GetCameraAVStreamManagementCluster()->GetLocalVideoRecordingEnabled());
        break;
    }
    case LocalSnapshotRecordingEnabled::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetLocalSnapshotRecordingEnabled(
            GetCameraAVStreamManagementCluster()->GetLocalSnapshotRecordingEnabled());
        break;
    }
    case StatusLightEnabled::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetStatusLightEnabled(
            GetCameraAVStreamManagementCluster()->GetStatusLightEnabled());
        break;
    }
    case ImageRotation::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetImageRotation(GetCameraAVStreamManagementCluster()->GetImageRotation());
        break;
    }
    case ImageFlipHorizontal::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetImageFlipHorizontal(
            GetCameraAVStreamManagementCluster()->GetImageFlipHorizontal());
        break;
    }
    case ImageFlipVertical::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetImageFlipVertical(
            GetCameraAVStreamManagementCluster()->GetImageFlipVertical());
        break;
    }
    default:
        ChipLogProgress(Camera, "Unknown Attribute with AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
    }
}

Protocols::InteractionModel::Status CameraAVStreamManager::CaptureSnapshot(const Nullable<uint16_t> streamID,
                                                                           const VideoResolutionStruct & resolution,
                                                                           ImageSnapshot & outImageSnapshot)
{
    if (mCameraDeviceHAL->GetCameraHALInterface().CaptureSnapshot(streamID, resolution, outImageSnapshot) == CameraError::SUCCESS)
    {
        return Status::Success;
    }
    else
    {
        return Status::Failure;
    }
}

CHIP_ERROR
CameraAVStreamManager::AllocatedVideoStreamsLoaded()
{
    const std::vector<VideoStreamStruct> & persistedStreams = GetCameraAVStreamManagementCluster()->GetAllocatedVideoStreams();
    auto & halStreams                                       = mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams();

    for (auto & halStream : halStreams)
    {
        auto it = std::find_if(persistedStreams.begin(), persistedStreams.end(), [&](const VideoStreamStruct & persistedStream) {
            return persistedStream.videoStreamID == halStream.videoStreamParams.videoStreamID;
        });

        if (it != persistedStreams.end())
        {
            // Found in persisted streams, mark as allocated in HAL
            halStream.isAllocated = true;
            ChipLogProgress(Camera, "HAL Video Stream ID %u marked as allocated from persisted state.",
                            halStream.videoStreamParams.videoStreamID);

            // Signal for starting the video stream
            OnVideoStreamAllocated(*it, StreamAllocationAction::kNewAllocation);
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManager::AllocatedAudioStreamsLoaded()
{
    const std::vector<AudioStreamStruct> & persistedStreams = GetCameraAVStreamManagementCluster()->GetAllocatedAudioStreams();
    auto & halStreams                                       = mCameraDeviceHAL->GetCameraHALInterface().GetAvailableAudioStreams();

    for (auto & halStream : halStreams)
    {
        auto it = std::find_if(persistedStreams.begin(), persistedStreams.end(), [&](const AudioStreamStruct & persistedStream) {
            return persistedStream.audioStreamID == halStream.audioStreamParams.audioStreamID;
        });

        if (it != persistedStreams.end())
        {
            // Found in persisted streams, mark as allocated in HAL
            halStream.isAllocated = true;
            ChipLogProgress(Camera, "HAL Audio Stream ID %u marked as allocated from persisted state.",
                            halStream.audioStreamParams.audioStreamID);

            // Start the audio stream from HAL for serving.
            if (mCameraDeviceHAL->GetCameraHALInterface().StartAudioStream(halStream.audioStreamParams.audioStreamID) !=
                CameraError::SUCCESS)
            {
                ChipLogError(Camera, "Failed to start HAL Audio Stream for persisted ID %u.",
                             halStream.audioStreamParams.audioStreamID);
            }
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManager::AllocatedSnapshotStreamsLoaded()
{
    const std::vector<SnapshotStreamStruct> & persistedStreams =
        GetCameraAVStreamManagementCluster()->GetAllocatedSnapshotStreams();
    auto & halStreams = mCameraDeviceHAL->GetCameraHALInterface().GetAvailableSnapshotStreams();
    std::set<uint16_t> halStreamIds;

    // Keep a set of the streamIDs provided by HAL
    for (const auto & halStream : halStreams)
    {
        halStreamIds.insert(halStream.snapshotStreamParams.snapshotStreamID);
    }

    for (auto & halStream : halStreams)
    {
        auto it = std::find_if(persistedStreams.begin(), persistedStreams.end(), [&](const SnapshotStreamStruct & persistedStream) {
            return persistedStream.snapshotStreamID == halStream.snapshotStreamParams.snapshotStreamID;
        });

        if (it != persistedStreams.end())
        {
            // Found in persisted streams, mark as allocated in HAL
            halStream.isAllocated = true;

            ChipLogProgress(Camera, "HAL Snapshot Stream ID %u marked as allocated from persisted state.",
                            halStream.snapshotStreamParams.snapshotStreamID);

            // Start the snapshot stream for serving.
            if (mCameraDeviceHAL->GetCameraHALInterface().StartSnapshotStream(halStream.snapshotStreamParams.snapshotStreamID) !=
                CameraError::SUCCESS)
            {
                ChipLogError(Camera, "Failed to start HAL Snapshot Stream for persisted ID %u.",
                             halStream.snapshotStreamParams.snapshotStreamID);
            }
        }
    }

    // Allocate missing Persisted Streams

    for (const auto & persistedStream : persistedStreams)
    {
        if (halStreamIds.find(persistedStream.snapshotStreamID) == halStreamIds.end())
        {
            ChipLogProgress(Camera, "Persisted Snapshot Stream ID %u not found in HAL, attempting to allocate.",
                            persistedStream.snapshotStreamID);

            // Convert SnapshotStreamStruct to SnapshotStreamAllocateArgs
            CameraAVStreamManagementDelegate::SnapshotStreamAllocateArgs snapshotStreamArgs;

            snapshotStreamArgs.imageCodec       = persistedStream.imageCodec;
            snapshotStreamArgs.maxFrameRate     = persistedStream.frameRate;
            snapshotStreamArgs.minResolution    = persistedStream.minResolution;
            snapshotStreamArgs.maxResolution    = persistedStream.maxResolution;
            snapshotStreamArgs.quality          = persistedStream.quality;
            snapshotStreamArgs.encodedPixels    = false;
            snapshotStreamArgs.hardwareEncoder  = false;
            snapshotStreamArgs.watermarkEnabled = persistedStream.watermarkEnabled;
            snapshotStreamArgs.OSDEnabled       = persistedStream.OSDEnabled;

            uint16_t streamID = persistedStream.snapshotStreamID; // Use the persisted ID

            CameraError halErr = mCameraDeviceHAL->GetCameraHALInterface().AllocateSnapshotStream(snapshotStreamArgs, streamID);

            if (halErr == CameraError::SUCCESS)
            {
                ChipLogProgress(Camera, "Successfully allocated HAL Snapshot Stream for persisted ID %u.", streamID);
            }
            else
            {
                ChipLogError(Camera, "Failed to allocate HAL Snapshot Stream for persisted ID %u. HAL Error: %d",
                             persistedStream.snapshotStreamID, static_cast<int>(halErr));
                return CHIP_ERROR_INTERNAL;
            }
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManager::PersistentAttributesLoadedCallback()
{
    ChipLogDetail(Camera, "Successfully loaded persistent attributes");

    CHIP_ERROR err = AllocatedVideoStreamsLoaded();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Allocated video streams could not be loaded: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    err = AllocatedAudioStreamsLoaded();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Allocated audio streams could not be loaded: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    err = AllocatedSnapshotStreamsLoaded();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Allocated snapshot streams could not be loaded: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManager::OnTransportAcquireAudioVideoStreams(uint16_t audioStreamID, uint16_t videoStreamID)
{
    // Update the available audio stream in the HAL
    for (AudioStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableAudioStreams())
    {
        if (stream.audioStreamParams.audioStreamID == audioStreamID && stream.isAllocated)
        {
            if (stream.audioStreamParams.referenceCount < UINT8_MAX)
            {
                stream.audioStreamParams.referenceCount++;
            }
            else
            {
                ChipLogError(Camera, "Attempted to increment audio stream %u ref count beyond max limit", audioStreamID);
            }
        }
    }

    // Update the available video stream in the HAL
    for (VideoStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams())
    {
        if (stream.videoStreamParams.videoStreamID == videoStreamID && stream.isAllocated)
        {
            if (stream.videoStreamParams.referenceCount < UINT8_MAX)
            {
                stream.videoStreamParams.referenceCount++;
            }
            else
            {
                ChipLogError(Camera, "Attempted to increment video stream %u ref count beyond max limit", videoStreamID);
            }
        }
    }

    // Update the counts in the SDK allocated stream attributes
    if (GetCameraAVStreamManagementCluster()->UpdateAudioStreamRefCount(audioStreamID, /* shouldIncrement = */ true) !=
        CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to increment audio stream %u ref count in SDK", audioStreamID);
    }

    if (GetCameraAVStreamManagementCluster()->UpdateVideoStreamRefCount(videoStreamID, /* shouldIncrement = */ true) !=
        CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to increment video stream %u ref count in SDK", videoStreamID);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManager::OnTransportReleaseAudioVideoStreams(uint16_t audioStreamID, uint16_t videoStreamID)
{
    // Update the available audio stream in the HAL
    for (AudioStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableAudioStreams())
    {
        if (stream.audioStreamParams.audioStreamID == audioStreamID && stream.isAllocated)
        {
            if (stream.audioStreamParams.referenceCount > 0)
            {
                stream.audioStreamParams.referenceCount--;
            }
            else
            {
                ChipLogError(Camera, "Attempted to decrement audio stream %u ref count when it was already 0", audioStreamID);
            }
        }
    }

    // Update the available video stream in the HAL
    for (VideoStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams())
    {
        if (stream.videoStreamParams.videoStreamID == videoStreamID && stream.isAllocated)
        {
            if (stream.videoStreamParams.referenceCount > 0)
            {
                stream.videoStreamParams.referenceCount--;
            }
            else
            {
                ChipLogError(Camera, "Attempted to decrement video stream %u ref count when it was already 0", videoStreamID);
            }
        }
    }

    // Update the counts in the SDK allocated stream attributes
    if (GetCameraAVStreamManagementCluster()->UpdateAudioStreamRefCount(audioStreamID, /* shouldIncrement = */ false) !=
        CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to decrement audio stream %u ref count in SDK", audioStreamID);
    }

    if (GetCameraAVStreamManagementCluster()->UpdateVideoStreamRefCount(videoStreamID, /* shouldIncrement = */ false) !=
        CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to decrement video stream %u ref count in SDK", videoStreamID);
    }

    return CHIP_NO_ERROR;
}
