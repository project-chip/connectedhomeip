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

#include "camera-device.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/camera-av-stream-management-server/camera-av-stream-management-server.h>
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
using namespace Camera;
using chip::Protocols::InteractionModel::Status;

Protocols::InteractionModel::Status CameraAVStreamManager::VideoStreamAllocate(const VideoStreamStruct & allocateArgs,
                                                                               uint16_t & outStreamID)
{
    outStreamID               = kInvalidStreamID;
    bool foundAvailableStream = false;
    auto videoStreams         = CameraDevice::GetInstance().GetAvailableVideoStreams();

    for (VideoStream & stream : videoStreams)
    {
        if (!stream.isAllocated)
        {
            foundAvailableStream = true;

            if (stream.codec == allocateArgs.videoCodec && stream.frameRate >= allocateArgs.minFrameRate &&
                stream.frameRate <= allocateArgs.maxFrameRate && stream.videoRes.width >= allocateArgs.minResolution.width &&
                stream.videoRes.height >= allocateArgs.minResolution.height &&
                stream.videoRes.width <= allocateArgs.maxResolution.width &&
                stream.videoRes.height <= allocateArgs.maxResolution.height)
            {
                stream.isAllocated = true;
                outStreamID        = stream.id;

                // Start the video stream from HAL for serving.
                CameraDevice::GetInstance().StartVideoStream(stream.id);

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
                                                                             const chip::Optional<bool> waterMarkEnabled,
                                                                             const chip::Optional<bool> osdEnabled)
{
    // TODO : Needs Change
    auto videoStreams = CameraDevice::GetInstance().GetAvailableVideoStreams();
    for (VideoStream & stream : videoStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            ChipLogError(Zcl, "Modified video stream with ID: %d", streamID);
            return Status::Success;
        }
    }

    ChipLogError(Zcl, "Allocated video stream with ID: %d not found", streamID);
    return Status::Failure;
}

Protocols::InteractionModel::Status CameraAVStreamManager::VideoStreamDeallocate(const uint16_t streamID)
{
    auto videoStreams = CameraDevice::GetInstance().GetAvailableVideoStreams();
    for (VideoStream & stream : videoStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            // Stop the video stream
            CameraDevice::GetInstance().StopVideoStream(stream.id);

            stream.isAllocated = false;

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
    auto audioStreams         = CameraDevice::GetInstance().GetAvailableAudioStreams();

    for (AudioStream & stream : audioStreams)
    {
        if (!stream.isAllocated)
        {
            foundAvailableStream = true;

            if (stream.codec == allocateArgs.audioCodec && stream.channelCount == allocateArgs.channelCount &&
                stream.sampleRate == allocateArgs.sampleRate)
            {
                stream.isAllocated = true;
                outStreamID        = stream.id;

                // Start the audio stream from HAL for serving.
                CameraDevice::GetInstance().StartAudioStream(stream.id);

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
    auto audioStreams = CameraDevice::GetInstance().GetAvailableAudioStreams();
    for (AudioStream & stream : audioStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            // Stop the audio stream
            CameraDevice::GetInstance().StopAudioStream(stream.id);

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
    auto snapshotStreams      = CameraDevice::GetInstance().GetAvailableSnapshotStreams();

    for (SnapshotStream & stream : snapshotStreams)
    {
        if (!stream.isAllocated)
        {
            foundAvailableStream = true;

            if (stream.codec == allocateArgs.imageCodec && stream.quality == allocateArgs.quality &&
                stream.frameRate <= allocateArgs.frameRate && stream.videoRes.width >= allocateArgs.minResolution.width &&
                stream.videoRes.height >= allocateArgs.minResolution.height &&
                stream.videoRes.width <= allocateArgs.maxResolution.width &&
                stream.videoRes.height <= allocateArgs.maxResolution.height)
            {
                stream.isAllocated = true;
                outStreamID        = stream.id;

                // Start the snapshot stream for serving.
                CameraDevice::GetInstance().StartSnapshotStream(stream.id);

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
                                                                                const chip::Optional<bool> waterMarkEnabled,
                                                                                const chip::Optional<bool> osdEnabled)
{
    // TODO : change
    auto snapshotStreams = CameraDevice::GetInstance().GetAvailableSnapshotStreams();
    for (SnapshotStream & stream : snapshotStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            ChipLogError(Zcl, "Modified snapshot stream with ID: %d", streamID);
            return Status::Success;
        }
    }

    ChipLogError(Zcl, "Allocated snapshot stream with ID: %d not found", streamID);
    return Status::Failure;
}

Protocols::InteractionModel::Status CameraAVStreamManager::SnapshotStreamDeallocate(const uint16_t streamID)
{
    auto snapshotStreams = CameraDevice::GetInstance().GetAvailableSnapshotStreams();
    for (SnapshotStream & stream : snapshotStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            // Stop the snapshot stream for serving.
            CameraDevice::GetInstance().StopSnapshotStream(stream.id);

            stream.isAllocated = false;
            break;
        }
    }

    return Status::Success;
}

void CameraAVStreamManager::OnRankedStreamPrioritiesChanged()
{
    ChipLogProgress(Zcl, "Ranked stream priorities changed");
}

void CameraAVStreamManager::OnAttributeChanged(AttributeId attributeId)
{
    ChipLogProgress(Zcl, "Attribute changed for AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
}

Protocols::InteractionModel::Status CameraAVStreamManager::CaptureSnapshot(const uint16_t streamID,
                                                                           const VideoResolutionStruct & resolution,
                                                                           ImageSnapshot & outImageSnapshot)
{
    if (CameraDevice::GetInstance().CaptureSnapshot(streamID, resolution, outImageSnapshot) == CameraError::SUCCESS)
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
    ChipLogError(Zcl, "Persistent attributes loaded");

    return CHIP_NO_ERROR;
}
