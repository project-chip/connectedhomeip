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
    outStreamID               = kInvalidStreamID;
    bool foundAvailableStream = false;

    for (VideoStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams())
    {
        if (stream.IsCompatible(allocateArgs))
        {
            foundAvailableStream = true;

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

    if (!foundAvailableStream)
    {
        return Status::ResourceExhausted;
    }

    return Status::Failure;
}

Protocols::InteractionModel::Status CameraAVStreamManager::VideoStreamModify(const uint16_t streamID,
                                                                             const chip::Optional<bool> waterMarkEnabled,
                                                                             const chip::Optional<bool> osdEnabled)
{
    for (VideoStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableVideoStreams())
    {
        if (stream.videoStreamParams.videoStreamID == streamID && stream.isAllocated)
        {
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
    return Status::Failure;
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

            break;
        }
    }

    return Status::Success;
}

Protocols::InteractionModel::Status CameraAVStreamManager::AudioStreamAllocate(const AudioStreamStruct & allocateArgs,
                                                                               uint16_t & outStreamID)
{
    outStreamID               = kInvalidStreamID;
    bool foundAvailableStream = false;

    for (AudioStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableAudioStreams())
    {
        if (stream.IsCompatible(allocateArgs))
        {
            foundAvailableStream = true;

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

    if (!foundAvailableStream)
    {
        return Status::ResourceExhausted;
    }

    return Status::Failure;
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
            break;
        }
    }

    return Status::Success;
}

Protocols::InteractionModel::Status CameraAVStreamManager::SnapshotStreamAllocate(const SnapshotStreamStruct & allocateArgs,
                                                                                  uint16_t & outStreamID)
{
    outStreamID               = kInvalidStreamID;
    bool foundAvailableStream = false;

    for (SnapshotStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableSnapshotStreams())
    {
        if (stream.IsCompatible(allocateArgs))
        {
            foundAvailableStream = true;

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

    if (!foundAvailableStream)
    {
        return Status::ResourceExhausted;
    }

    return Status::Failure;
}

Protocols::InteractionModel::Status CameraAVStreamManager::SnapshotStreamModify(const uint16_t streamID,
                                                                                const chip::Optional<bool> waterMarkEnabled,
                                                                                const chip::Optional<bool> osdEnabled)
{
    for (SnapshotStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableSnapshotStreams())
    {
        if (stream.snapshotStreamParams.snapshotStreamID == streamID && stream.isAllocated)
        {
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
    return Status::Failure;
}

Protocols::InteractionModel::Status CameraAVStreamManager::SnapshotStreamDeallocate(const uint16_t streamID)
{
    for (SnapshotStream & stream : mCameraDeviceHAL->GetCameraHALInterface().GetAvailableSnapshotStreams())
    {
        if (stream.snapshotStreamParams.snapshotStreamID == streamID && stream.isAllocated)
        {
            // Stop the snapshot stream for serving.
            mCameraDeviceHAL->GetCameraHALInterface().StopSnapshotStream(streamID);

            stream.isAllocated = false;
            break;
        }
    }

    return Status::Success;
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
