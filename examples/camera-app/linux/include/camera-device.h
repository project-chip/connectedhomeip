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

#include "default-media-controller.h"
#include "network-stream-source.h"
#include <protocols/interaction_model/StatusCode.h>

#include <gst/gst.h>
#define STREAM_GST_DEST_IP "127.0.0.1"
#define VIDEO_STREAM_GST_DEST_PORT 5000
#define AUDIO_STREAM_GST_DEST_PORT 5001
// TODO: Define a configuration flag and enable/disable during the build. Configure this after the controller/TH side UI is ready.
// Enable to use test src instead of hardware source for testing purposes.
// #define AV_STREAM_GST_USE_TEST_SRC

// Camera Constraints set to typical values.
// TODO: Look into ways to fetch from hardware, if required/possible.
static constexpr uint32_t kMaxContentBufferSizeBytes = 4096;
static constexpr uint32_t kMaxNetworkBandwidthMbps   = 128;
static constexpr uint8_t kMaxConcurrentEncoders      = 1;
static constexpr uint32_t kMaxEncodedPixelRate       = 27648000; // 720p at 30fps
static constexpr uint8_t kMicrophoneMinLevel         = 1;
static constexpr uint8_t kMicrophoneMaxLevel         = 254;  // Spec constraint
static constexpr uint8_t kMicrophoneMaxChannelCount  = 8;    // Spec Constraint in AudioStreamAllocate
static constexpr uint16_t kMinResolutionWidth        = 256;  // Low SD resolution
static constexpr uint16_t kMinResolutionHeight       = 144;  // Low SD resolution
static constexpr uint16_t kMaxResolutionWidth        = 1920; // 1080p resolution
static constexpr uint16_t kMaxResolutionHeight       = 1080; // 1080p resolution
static constexpr uint16_t kSnapshotStreamFrameRate   = 30;
static constexpr uint16_t kMaxVideoFrameRate         = 120;
static constexpr uint16_t kMinVideoFrameRate         = 15;
static constexpr uint32_t kMinBitRateBps             = 10000;   // 10 kbps
static constexpr uint32_t kMaxBitRateBps             = 2000000; // 2 mbps
static constexpr uint32_t kMinFragLenMsec            = 1000;    // 1 sec
static constexpr uint32_t kMaxFragLenMsec            = 10000;   // 10 sec
static constexpr uint16_t kVideoSensorWidthPixels    = 1920;    // 1080p resolution
static constexpr uint16_t kVideoSensorHeightPixels   = 1080;    // 1080p resolution

#define INVALID_SPKR_LEVEL (0)

namespace Camera {

// Camera defined constants for Pan, Tilt, Zoom bounding values
constexpr int16_t kMinPanValue  = -90;
constexpr int16_t kMaxPanValue  = 90;
constexpr int16_t kMinTiltValue = -90;
constexpr int16_t kMaxTiltValue = 90;
constexpr uint8_t kMaxZoomValue = 75;

class CameraDevice : public CameraDeviceInterface, public CameraDeviceInterface::CameraHALInterface
{
public:
    chip::app::Clusters::ChimeDelegate & GetChimeDelegate();
    chip::app::Clusters::WebRTCTransportProvider::Delegate & GetWebRTCProviderDelegate();
    chip::app::Clusters::CameraAvStreamManagement::CameraAVStreamMgmtDelegate & GetCameraAVStreamMgmtDelegate();
    chip::app::Clusters::CameraAvSettingsUserLevelManagement::Delegate & GetCameraAVSettingsUserLevelMgmtDelegate();

    MediaController & GetMediaController();

    CameraDevice();
    ~CameraDevice();

    CameraDeviceInterface::CameraHALInterface & GetCameraHALInterface() { return *this; }

    // HAL interface impl
    CameraError InitializeCameraDevice() override;

    CameraError InitializeStreams() override;

    CameraError CaptureSnapshot(const chip::app::DataModel::Nullable<uint16_t> streamID, const VideoResolutionStruct & resolution,
                                ImageSnapshot & outImageSnapshot) override;

    CameraError StartVideoStream(uint16_t streamID) override;

    // Stop video stream
    CameraError StopVideoStream(uint16_t streamID) override;

    // Start audio stream
    CameraError StartAudioStream(uint16_t streamID) override;

    // Stop audio stream
    CameraError StopAudioStream(uint16_t streamID) override;

    // Start snapshot stream
    CameraError StartSnapshotStream(uint16_t streamID) override;

    // Stop snapshot stream
    CameraError StopSnapshotStream(uint16_t streamID) override;

    uint8_t GetMaxConcurrentEncoders() override;

    uint32_t GetMaxEncodedPixelRate() override;

    VideoSensorParamsStruct & GetVideoSensorParams() override;

    bool GetCameraSupportsNightVision() override;

    bool GetNightVisionUsesInfrared() override;

    VideoResolutionStruct & GetMinViewport() override;

    std::vector<RateDistortionTradeOffStruct> & GetRateDistortionTradeOffPoints() override;

    uint32_t GetMaxContentBufferSize() override;

    AudioCapabilitiesStruct & GetMicrophoneCapabilities() override;

    AudioCapabilitiesStruct & GetSpeakerCapabilities() override;

    std::vector<SnapshotCapabilitiesStruct> & GetSnapshotCapabilities() override;

    uint32_t GetMaxNetworkBandwidth() override;

    uint16_t GetCurrentFrameRate() override;

    CameraError SetHDRMode(bool hdrMode) override;
    bool GetHDRMode() override { return mHDREnabled; }

    std::vector<StreamUsageEnum> & GetSupportedStreamUsages() override;

    std::vector<StreamUsageEnum> & GetRankedStreamPriorities() override { return mRankedStreamPriorities; }

    // Sets the Default Camera Viewport
    CameraError SetViewport(const ViewportStruct & viewPort) override;
    const ViewportStruct & GetViewport() override { return mViewport; }

    /**
     * Sets the Viewport for a specific stream. The implementation of this HAL API is responsible
     * for updating the stream identified with the provided viewport. The invoker of this
     * API shall have already ensured that the provided viewport conforms to the specification
     * requirements on size and aspect ratio.
     *
     * @param stream   the currently allocated video stream on which the viewport is being set
     * @param viewport the viewport to be set on the stream
     */
    CameraError SetViewport(VideoStream & stream, const ViewportStruct & viewport);

    // Currently, defaulting to not supporting speaker.
    bool HasSpeaker() override { return false; }

    // Mute/Unmute speaker.
    CameraError SetSpeakerMuted(bool muteSpeaker) override { return CameraError::ERROR_NOT_IMPLEMENTED; }

    // Set speaker volume level.
    CameraError SetSpeakerVolume(uint8_t speakerVol) override { return CameraError::ERROR_NOT_IMPLEMENTED; }

    // Get the speaker max and min levels.
    uint8_t GetSpeakerMaxLevel() override { return INVALID_SPKR_LEVEL; }
    uint8_t GetSpeakerMinLevel() override { return INVALID_SPKR_LEVEL; }

    // Does camera have a microphone
    bool HasMicrophone() override { return true; }

    // Mute/Unmute microphone.
    CameraError SetMicrophoneMuted(bool muteMicrophone) override;
    bool GetMicrophoneMuted() override { return mMicrophoneMuted; }

    // Set microphone volume level.
    CameraError SetMicrophoneVolume(uint8_t microphoneVol) override;
    uint8_t GetMicrophoneVolume() override { return mMicrophoneVol; }

    // Get the microphone max and min levels.
    uint8_t GetMicrophoneMaxLevel() override { return kMicrophoneMaxLevel; }
    uint8_t GetMicrophoneMinLevel() override { return kMicrophoneMinLevel; }

    int16_t GetPanMin();

    int16_t GetPanMax();

    int16_t GetTiltMin();

    int16_t GetTiltMax();

    uint8_t GetZoomMax();

    CameraError SetPan(int16_t aPan);
    CameraError SetTilt(int16_t aTilt);
    CameraError SetZoom(uint8_t aZoom);

    std::vector<VideoStream> & GetAvailableVideoStreams() override { return videoStreams; }

    std::vector<AudioStream> & GetAvailableAudioStreams() override { return audioStreams; }

    std::vector<SnapshotStream> & GetAvailableSnapshotStreams() override { return snapshotStreams; }

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

    DefaultMediaController mMediaController;

    uint16_t mPan  = chip::app::Clusters::CameraAvSettingsUserLevelManagement::kDefaultPan;
    uint16_t mTilt = chip::app::Clusters::CameraAvSettingsUserLevelManagement::kDefaultTilt;
    int8_t mZoom   = chip::app::Clusters::CameraAvSettingsUserLevelManagement::kDefaultZoom;
    // Use a standard 1080p aspect ratio
    chip::app::Clusters::CameraAvStreamManagement::ViewportStruct mViewport = { 320, 585, 2240, 1665 };
    uint16_t mCurrentVideoFrameRate                                         = 0;
    bool mHDREnabled                                                        = false;
    bool mMicrophoneMuted                                                   = false;
    uint8_t mMicrophoneMinLevel                                             = kMicrophoneMinLevel;
    uint8_t mMicrophoneMaxLevel                                             = kMicrophoneMaxLevel;
    uint8_t mMicrophoneVol                                                  = kMicrophoneMinLevel;

    std::vector<StreamUsageEnum> mRankedStreamPriorities = { StreamUsageEnum::kLiveView, StreamUsageEnum::kRecording };
};

} // namespace Camera
