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

#pragma once
#include "camera-av-stream-manager.h"
#include "camera-device-interface.h"
#include "chime-manager.h"
#include "webrtc-provider-manager.h"

#include "media-controller.h"
#include "network-stream-source.h"

#include <protocols/interaction_model/StatusCode.h>

#include <gst/gst.h>
#define STREAM_GST_DEST_IP "127.0.0.1"
#define VIDEO_STREAM_GST_DEST_PORT 5000
#define AUDIO_STREAM_GST_DEST_PORT 5001

namespace Camera {

class CameraDevice : public CameraDeviceInterface, public CameraDeviceInterface::CameraHALInterface
{
public:
    chip::app::Clusters::ChimeDelegate & GetChimeDelegate();
    chip::app::Clusters::WebRTCTransportProvider::Delegate & GetWebRTCProviderDelegate();
    chip::app::Clusters::CameraAvStreamManagement::CameraAVStreamMgmtDelegate & GetCameraAVStreamMgmtDelegate();

    CameraDevice();
    ~CameraDevice();

    CameraDeviceInterface::CameraHALInterface & GetCameraHALInterface() { return *this; }

    // HAL interface impl
    CameraError InitializeCameraDevice();

    CameraError InitializeStreams();

    CameraError CaptureSnapshot(const uint16_t streamID, const VideoResolutionStruct & resolution,
                                ImageSnapshot & outImageSnapshot);

    CameraError StartVideoStream(uint16_t streamID);

    // Stop video stream
    CameraError StopVideoStream(uint16_t streamID);

    // Start audio stream
    CameraError StartAudioStream(uint16_t streamID);

    // Stop audio stream
    CameraError StopAudioStream(uint16_t streamID);

    // Start snapshot stream
    CameraError StartSnapshotStream(uint16_t streamID);

    // Stop snapshot stream
    CameraError StopSnapshotStream(uint16_t streamID);

    VideoSensorParamsStruct & GetVideoSensorParams();

    bool GetNightVisionCapable();

    VideoResolutionStruct & GetMinViewport();

    uint8_t GetMaxConcurrentVideoEncoders();

    uint32_t GetMaxEncodedPixelRate();

    uint16_t GetFrameRate();

    void SetHDRMode(bool hdrMode);

    std::vector<VideoStream> & GetAvailableVideoStreams() { return videoStreams; }

    std::vector<AudioStream> & GetAvailableAudioStreams() { return audioStreams; }

    std::vector<SnapshotStream> & GetAvailableSnapshotStreams() { return snapshotStreams; }

private:
    int videoDeviceFd = -1;
    std::vector<VideoStream> videoStreams;       // Vector to hold available video streams
    std::vector<AudioStream> audioStreams;       // Vector to hold available audio streams
    std::vector<SnapshotStream> snapshotStreams; // Vector to hold available snapshot streams

    void InitializeVideoStreams();
    void InitializeAudioStreams();
    void InitializeSnapshotStreams();

    GstElement * CreateVideoPipeline(const std::string & device, int width, int height, int framerate, CameraError & error);
    GstElement * CreateAudioPipeline(const std::string & device, int channels, int sampleRate, CameraError & error);
    GstElement * CreateSnapshotPipeline(const std::string & device, int width, int height, int quality, int frameRate,
                                        const std::string & filename, CameraError & error);
    CameraError SetV4l2Control(uint32_t controlId, int value);

    // Various cluster server delegates
    ChimeManager mChimeManager;
    WebRTCProviderManager mWebRTCProviderManager;

    chip::app::Clusters::CameraAvStreamManagement::CameraAVStreamManager mCameraAVStreamManager;

    NetworkStreamSource mNetworkVideoSource;
    NetworkStreamSource mNetworkAudioSource;
    MediaController mMediaController;
};

} // namespace Camera
