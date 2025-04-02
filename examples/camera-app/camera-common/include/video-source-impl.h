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

#ifndef VIDEO_SOURCE_IMPL_H
#define VIDEO_SOURCE_IMPL_H

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/config.h>
#include <cstring>
#include <protocols/interaction_model/StatusCode.h>
#include <utility>

#include "camera-device-interface.h"

class VideoSourceImpl : public CameraHALInterface
{
public:
    // Destructor
    ~VideoSourceImpl() override = default;

    // Initialize the camera hardware
    CameraError InitializeCameraDevice() override;

    CameraError InitializeStreams() override;

    // Implement other video-related APIs as defined in CameraHALInterface

    CameraError VideoStreamAllocate(const VideoStreamStruct & videoStreamParams, uint16_t & outStreamID) override;

    CameraError AudioStreamAllocate(const AudioStreamStruct & audioStreamParams, uint16_t & outStreamID) override;

    CameraError SnapshotStreamAllocate(const SnapshotStreamStruct & snapshotStreamParams, uint16_t & outStreamID) override;

    CameraError VideoStreamDeallocate(uint16_t streamID) override;

    CameraError AudioStreamDeallocate(uint16_t streamID) override;

    CameraError SnapshotStreamDeallocate(uint16_t streamID) override;

    CameraError CaptureSnapshot(uint16_t streamID, const VideoResolutionStruct & resolution,
                                ImageSnapshot & outImageSnapshot) override;

    CameraError StartVideoStream(uint16_t streamID) override;

    CameraError StopVideoStream(uint16_t streamID) override;

    CameraError StartAudioStream(uint16_t streamID) override;

    CameraError StopAudioStream(uint16_t streamID) override;

    CameraError StartSnapshotStream(uint16_t streamID) override;

    CameraError StopSnapshotStream(uint16_t streamID) override;

    VideoSensorParamsStruct & GetVideoSensorParams() override;

    bool GetNightVisionCapable() override;

    VideoResolutionStruct & GetMinViewport() override;

    uint8_t GetMaxConcurrentVideoEncoders() override;

    uint32_t GetMaxEncodedPixelRate() override;

    uint16_t GetFrameRate() override;

    void SetHDRMode(bool hdrMode) override;

private:
    void * pipeline;
    VideoStreamStruct video_stream_params;
    std::chrono::system_clock start_time;
};

#endif // VIDEO_SOURCE_IMPL_H
