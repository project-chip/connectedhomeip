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

void CameraAVStreamManager::SetCameraDeviceHAL(CameraDeviceInterface * aCameraDeviceHAL)
{
    mCameraDeviceHAL = aCameraDeviceHAL;
}

Protocols::InteractionModel::Status CameraAVStreamManager::VideoStreamAllocate(const VideoStreamStruct & allocateArgs,
                                                                               uint16_t & outStreamID)
{
    outStreamID = kInvalidStreamID;

    for (VideoStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams())
    {
        if (stream.IsCompatible(allocateArgs))
        {
            outStreamID = stream.videoStreamParams.videoStreamID;
            if (!stream.isAllocated)
            {
                stream.isAllocated = true;

                // Start the video stream from HAL for serving.
                mCameraDeviceHAL->GetCameraHALInterface().StartVideoStream(outStreamID);

                // Set the default viewport on the newly allocated stream
                mCameraDeviceHAL->GetCameraHALInterface().SetViewport(stream,
                                                                      mCameraDeviceHAL->GetCameraHALInterface().GetViewport());

                // Inform DPTZ that there's an allocated stream
                mCameraDeviceHAL->GetCameraAVSettingsUserLevelMgmtDelegate().VideoStreamAllocated(outStreamID);

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
            if (stream.videoStreamParams.referenceCount > 0)
            {
                ChipLogError(Camera, "Video stream with ID: %d still in use", streamID);
                return Status::InvalidInState;
            }
            // Stop the video stream
            mCameraDeviceHAL->GetCameraHALInterface().StopVideoStream(streamID);

            stream.isAllocated = false;

            mCameraDeviceHAL->GetCameraAVSettingsUserLevelMgmtDelegate().VideoStreamDeallocated(streamID);

            return Status::Success;
        }
    }

    ChipLogError(Camera, "Allocated video stream with ID: %d not found", streamID);

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
            if (stream.audioStreamParams.referenceCount > 0)
            {
                ChipLogError(Camera, "Audio stream with ID: %d still in use", streamID);
                return Status::InvalidInState;
            }
            // Stop the audio stream
            mCameraDeviceHAL->GetCameraHALInterface().StopAudioStream(streamID);

            stream.isAllocated = false;

            return Status::Success;
        }
    }

    ChipLogError(Camera, "Allocated audio stream with ID: %d not found", streamID);

    return Status::NotFound;
}

Protocols::InteractionModel::Status CameraAVStreamManager::SnapshotStreamAllocate(const SnapshotStreamStruct & allocateArgs,
                                                                                  uint16_t & outStreamID)
{
    outStreamID = kInvalidStreamID;

    for (SnapshotStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableSnapshotStreams())
    {
        if (stream.IsCompatible(allocateArgs))
        {
            outStreamID = stream.snapshotStreamParams.snapshotStreamID;
            if (!stream.isAllocated)
            {
                stream.isAllocated = true;

                // Start the snapshot stream for serving.
                mCameraDeviceHAL->GetCameraHALInterface().StartSnapshotStream(outStreamID);

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

void CameraAVStreamManager::OnRankedStreamPrioritiesChanged()
{
    ChipLogProgress(Camera, "Ranked stream priorities changed");
}

void CameraAVStreamManager::OnAttributeChanged(AttributeId attributeId)
{
    ChipLogProgress(Camera, "Attribute changed for AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));

    switch (attributeId)
    {
    case HDRModeEnabled::Id: {

        mCameraDeviceHAL->GetCameraHALInterface().SetHDRMode(GetCameraAVStreamMgmtServer()->GetHDRModeEnabled());
        break;
    }
    case SoftRecordingPrivacyModeEnabled::Id: {
        break;
    }
    case SoftLivestreamPrivacyModeEnabled::Id: {
        break;
    }
    case NightVision::Id: {
        break;
    }
    case NightVisionIllum::Id: {
        break;
    }
    case Viewport::Id: {
        // Update the device default
        mCameraDeviceHAL->GetCameraHALInterface().SetViewport(GetCameraAVStreamMgmtServer()->GetViewport());

        // Update the per stream viewports on the camera
        for (VideoStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams())
        {
            mCameraDeviceHAL->GetCameraHALInterface().SetViewport(stream, GetCameraAVStreamMgmtServer()->GetViewport());
        }

        // Inform DPTZ (the server) that the camera default viewport has changed
        mCameraDeviceHAL->GetCameraAVSettingsUserLevelMgmtDelegate().DefaultViewportUpdated(
            GetCameraAVStreamMgmtServer()->GetViewport());
        break;
    }
    case SpeakerMuted::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetSpeakerMuted(GetCameraAVStreamMgmtServer()->GetSpeakerMuted());
        break;
    }
    case SpeakerVolumeLevel::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetSpeakerVolume(GetCameraAVStreamMgmtServer()->GetSpeakerVolumeLevel());
        break;
    }
    case MicrophoneMuted::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetMicrophoneMuted(GetCameraAVStreamMgmtServer()->GetMicrophoneMuted());
        break;
    }
    case MicrophoneVolumeLevel::Id: {
        mCameraDeviceHAL->GetCameraHALInterface().SetMicrophoneVolume(GetCameraAVStreamMgmtServer()->GetMicrophoneVolumeLevel());
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
    ChipLogError(Camera, "Persistent attributes loaded");

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
    if (GetCameraAVStreamMgmtServer()->UpdateAudioStreamRefCount(audioStreamID, /* shouldIncrement = */ true) != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to increment audio stream %u ref count in SDK", audioStreamID);
    }

    if (GetCameraAVStreamMgmtServer()->UpdateVideoStreamRefCount(videoStreamID, /* shouldIncrement = */ true) != CHIP_NO_ERROR)
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
    if (GetCameraAVStreamMgmtServer()->UpdateAudioStreamRefCount(audioStreamID, /* shouldIncrement = */ false) != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to decrement audio stream %u ref count in SDK", audioStreamID);
    }

    if (GetCameraAVStreamMgmtServer()->UpdateVideoStreamRefCount(videoStreamID, /* shouldIncrement = */ false) != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to decrement video stream %u ref count in SDK", videoStreamID);
    }

    return CHIP_NO_ERROR;
}
