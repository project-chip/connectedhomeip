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
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvStreamManagement;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvStreamManagement {

Protocols::InteractionModel::Status CameraAVStreamManager::VideoStreamAllocate(const VideoStreamAllocateArgs & allocateArgs,
                                                                               uint16_t & outStreamID)
{
    outStreamID = kInvalidStreamID;

    for (VideoStream & stream : videoStreams)
    {
        if (!stream.isAllocated && stream.codec == allocateArgs.videoCodec)
        {
            stream.isAllocated = true;
            outStreamID        = stream.id;
        }
    }

    return Status::Success;
}

Protocols::InteractionModel::Status CameraAVStreamManager::VideoStreamModify(const uint16_t streamID,
                                                                             const chip::Optional<bool> waterMarkEnabled,
                                                                             const chip::Optional<bool> osdEnabled)
{
    return Status::Success;
}

Protocols::InteractionModel::Status CameraAVStreamManager::VideoStreamDeallocate(const uint16_t streamID)
{
    for (VideoStream & stream : videoStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            stream.isAllocated = false;
        }
    }

    return Status::Success;
}

Protocols::InteractionModel::Status CameraAVStreamManager::AudioStreamAllocate(const AudioStreamAllocateArgs & allocateArgs,
                                                                               uint16_t & outStreamID)
{
    outStreamID = kInvalidStreamID;

    for (AudioStream & stream : audioStreams)
    {
        if (!stream.isAllocated && stream.codec == allocateArgs.audioCodec)
        {
            stream.isAllocated = true;
            outStreamID        = stream.id;
        }
    }

    return Status::Success;
}

Protocols::InteractionModel::Status CameraAVStreamManager::AudioStreamDeallocate(const uint16_t streamID)
{
    for (AudioStream & stream : audioStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            stream.isAllocated = false;
        }
    }

    return Status::Success;
}

Protocols::InteractionModel::Status CameraAVStreamManager::SnapshotStreamAllocate(const SnapshotStreamAllocateArgs & allocateArgs,
                                                                                  uint16_t & outStreamID)
{
    outStreamID = kInvalidStreamID;

    for (SnapshotStream & stream : snapshotStreams)
    {
        if (!stream.isAllocated && stream.codec == allocateArgs.imageCodec)
        {
            stream.isAllocated = true;
            outStreamID        = stream.id;
        }
    }

    return Status::Success;
}

Protocols::InteractionModel::Status CameraAVStreamManager::SnapshotStreamDeallocate(const uint16_t streamID)
{
    for (SnapshotStream & stream : snapshotStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            stream.isAllocated = false;
        }
    }

    return Status::Success;
}

void CameraAVStreamManager::OnRankedStreamPrioritiesChanged()
{
    return;
}

Protocols::InteractionModel::Status CameraAVStreamManager::CaptureSnapshot(const uint16_t streamID,
                                                                           const VideoResolutionStruct & resolution,
                                                                           ImageSnapshot & outImageSnapshot)
{
    return Status::Success;
}

CHIP_ERROR
CameraAVStreamManager::LoadAllocatedVideoStreams(std::vector<VideoStreamStruct> & allocatedVideoStreams)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManager::LoadAllocatedAudioStreams(std::vector<AudioStreamStruct> & allocatedAudioStreams)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManager::LoadAllocatedSnapshotStreams(std::vector<SnapshotStreamStruct> & allocatedSnapshotStreams)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR
CameraAVStreamManager::PersistentAttributesLoadedCallback()
{
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

} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
