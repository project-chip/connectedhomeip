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
#include "camera-avsettingsuserlevel-manager.h"
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

#define MAX_CONTENT_BUFFER_SIZE_BYTES (1024)
#define MAX_ENCODED_PIXEL_RATE (10000)
#define MAX_CONCURRENT_VIDEO_ENCODERS (1)
#define MAX_NETWORK_BANDWIDTH_MBPS (64)
#define MICROPHONE_MIN_LEVEL (1)
#define MICROPHONE_MAX_LEVEL (254)
#define INVALID_SPKR_LEVEL (0)

namespace Camera {

class CameraDevice : public CameraDeviceInterface, public CameraDeviceInterface::CameraHALInterface
{
public:
    chip::app::Clusters::ChimeDelegate & GetChimeDelegate();
    chip::app::Clusters::WebRTCTransportProvider::Delegate & GetWebRTCProviderDelegate();
    chip::app::Clusters::CameraAvStreamManagement::CameraAVStreamMgmtDelegate & GetCameraAVStreamMgmtDelegate();
    chip::app::Clusters::CameraAvSettingsUserLevelManagement::Delegate & GetCameraAVSettingsUserLevelMgmtDelegate();


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

    uint8_t GetMaxConcurrentVideoEncoders();

    uint32_t GetMaxEncodedPixelRate();

    VideoSensorParamsStruct & GetVideoSensorParams();

    bool GetNightVisionCapable();

    VideoResolutionStruct & GetMinViewport();

    uint32_t GetMaxContentBufferSize();

    uint32_t GetMaxNetworkBandwidth();

    uint16_t GetCurrentFrameRate();

    CameraError SetHDRMode(bool hdrMode);
    bool GetHDRMode() { return mHDREnabled; }

    CameraError SetViewport(const ViewportStruct & viewPort);
    const ViewportStruct & GetViewport() { return mViewport; }

    // Currently, defaulting to not supporting speaker.
    bool HasSpeaker() { return false; }

    // Mute/Unmute speaker.
    CameraError SetSpeakerMuted(bool muteSpeaker) { return CameraError::ERROR_NOT_IMPLEMENTED; }

    // Set speaker volume level.
    CameraError SetSpeakerVolume(uint8_t speakerVol) { return CameraError::ERROR_NOT_IMPLEMENTED; }

    // Get the speaker max and min levels.
    uint8_t GetSpeakerMaxLevel() { return INVALID_SPKR_LEVEL; }
    uint8_t GetSpeakerMinLevel() { return INVALID_SPKR_LEVEL; }

    // Does camera have a microphone
    bool HasMicrophone() { return true; }

    // Mute/Unmute microphone.
    CameraError SetMicrophoneMuted(bool muteMicrophone);
    bool GetMicrophoneMuted() { return mMicrophoneMuted; }

    // Set microphone volume level.
    CameraError SetMicrophoneVolume(uint8_t microphoneVol);
    uint8_t GetMicrophoneVolume() { return mMicrophoneVol; }

    // Get the microphone max and min levels.
    uint8_t GetMicrophoneMaxLevel() { return MICROPHONE_MAX_LEVEL; }
    uint8_t GetMicrophoneMinLevel() { return MICROPHONE_MIN_LEVEL; }

    int16_t GetPanMin();

    int16_t GetPanMax();

    int16_t GetTiltMin();

    int16_t GetTiltMax();

    uint8_t GetZoomMax();

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
    chip::app::Clusters::CameraAvSettingsUserLevelManagement::CameraAVSettingsUserLevelManager mCameraAVSettingsUserLevelManager;

    NetworkStreamSource mNetworkVideoSource;
    NetworkStreamSource mNetworkAudioSource;
    MediaController mMediaController;

    uint16_t mCurrentVideoFrameRate                                         = 0;
    bool mHDREnabled                                                        = false;
    bool mMicrophoneMuted                                                   = false;
    uint8_t mMicrophoneMinLevel                                             = MICROPHONE_MIN_LEVEL;
    uint8_t mMicrophoneMaxLevel                                             = MICROPHONE_MAX_LEVEL;
    uint8_t mMicrophoneVol                                                  = MICROPHONE_MIN_LEVEL;
    chip::app::Clusters::CameraAvStreamManagement::ViewportStruct mViewport = { 325, 585, 2244, 1664 };
};

} // namespace Camera
