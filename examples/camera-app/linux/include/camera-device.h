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
#include "push-av-stream-manager.h"
#include "webrtc-provider-manager.h"
#include "zone-manager.h"

#include "default-media-controller.h"
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
static constexpr uint8_t kSpeakerMinLevel            = 1;
static constexpr uint8_t kSpeakerMaxLevel            = 254;       // Spec constraint
static constexpr uint8_t kSpeakerMaxChannelCount     = 8;         // Same as Microphone
static constexpr uint32_t kMaxEncodedPixelRate       = 248832000; // 1080p at 120fps(1920 * 1080 * 120)
static constexpr uint8_t kMicrophoneMinLevel         = 1;
static constexpr uint8_t kMicrophoneMaxLevel         = 254;  // Spec constraint
static constexpr uint8_t kMicrophoneMaxChannelCount  = 8;    // Spec Constraint in AudioStreamAllocate
static constexpr uint16_t kMinResolutionWidth        = 640;  // Low SD resolution
static constexpr uint16_t kMinResolutionHeight       = 360;  // Low SD resolution
static constexpr uint16_t kMaxResolutionWidth        = 1920; // 1080p resolution
static constexpr uint16_t kMaxResolutionHeight       = 1080; // 1080p resolution
static constexpr uint16_t kSnapshotStreamFrameRate   = 30;
static constexpr uint16_t kMaxVideoFrameRate         = 120;
static constexpr uint16_t kMinVideoFrameRate         = 30;
static constexpr uint32_t kMinBitRateBps             = 10000;   // 10 kbps
static constexpr uint32_t kMaxBitRateBps             = 2000000; // 2 mbps
static constexpr uint32_t kKeyFrameIntervalMsec      = 4000;    // 4 sec; recommendation from Spec
static constexpr uint16_t kVideoSensorWidthPixels    = 1920;    // 1080p resolution
static constexpr uint16_t kVideoSensorHeightPixels   = 1080;    // 1080p resolution
static constexpr uint16_t kMinImageRotation          = 0;
static constexpr uint16_t kMaxImageRotation          = 359; // Spec constraint
static constexpr uint8_t kMaxZones                   = 10;  // Spec has min 1
static constexpr uint8_t kMaxUserDefinedZones        = 10;  // Spec has min 5
static constexpr uint8_t kSensitivityMax             = 10;  // Spec has 2 to 10

#define INVALID_SPKR_LEVEL (0)

namespace Camera {

constexpr const char * kDefaultVideoDevicePath = "/dev/video0";

// Camera defined constants for Pan, Tilt, Zoom bounding values
constexpr int16_t kMinPanValue  = -90;
constexpr int16_t kMaxPanValue  = 90;
constexpr int16_t kMinTiltValue = -90;
constexpr int16_t kMaxTiltValue = 90;
constexpr uint8_t kMaxZoomValue = 75;

class CameraDevice : public CameraDeviceInterface, public CameraDeviceInterface::CameraHALInterface
{
public:
    chip::app::Clusters::ChimeDelegate & GetChimeDelegate() override;
    chip::app::Clusters::WebRTCTransportProvider::Delegate & GetWebRTCProviderDelegate() override;
    chip::app::Clusters::CameraAvStreamManagement::CameraAVStreamMgmtDelegate & GetCameraAVStreamMgmtDelegate() override;
    chip::app::Clusters::CameraAvStreamManagement::CameraAVStreamController & GetCameraAVStreamMgmtController() override;
    chip::app::Clusters::CameraAvSettingsUserLevelManagement::Delegate & GetCameraAVSettingsUserLevelMgmtDelegate() override;
    chip::app::Clusters::PushAvStreamTransportDelegate & GetPushAVTransportDelegate() override;
    chip::app::Clusters::ZoneManagement::Delegate & GetZoneManagementDelegate() override;

    MediaController & GetMediaController() override;

    CameraDevice();
    ~CameraDevice();

    CameraDeviceInterface::CameraHALInterface & GetCameraHALInterface() override { return *this; }

    void Init();

    // HAL interface impl
    CameraError InitializeCameraDevice() override;

    CameraError InitializeStreams() override;

    CameraError CaptureSnapshot(const chip::app::DataModel::Nullable<uint16_t> streamID, const VideoResolutionStruct & resolution,
                                ImageSnapshot & outImageSnapshot) override;

    CameraError StartVideoStream(const VideoStreamStruct & allocatedStream) override;

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

    bool GetCameraSupportsHDR() override;

    bool GetCameraSupportsNightVision() override;

    bool GetNightVisionUsesInfrared() override;

    bool GetCameraSupportsWatermark() override;

    bool GetCameraSupportsOSD() override;

    bool GetCameraSupportsSoftPrivacy() override;

    bool GetCameraSupportsImageControl() override;

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

    bool GetHardPrivacyMode() override { return mHardPrivacyModeOn; }

    CameraError SetNightVision(chip::app::Clusters::CameraAvStreamManagement::TriStateAutoEnum nightVision) override;
    chip::app::Clusters::CameraAvStreamManagement::TriStateAutoEnum GetNightVision() override { return mNightVision; }

    std::vector<StreamUsageEnum> & GetSupportedStreamUsages() override;

    std::vector<StreamUsageEnum> & GetStreamUsagePriorities() override { return mStreamUsagePriorities; }

    // Sets the Default Camera Viewport
    CameraError SetViewport(const chip::app::Clusters::Globals::Structs::ViewportStruct::Type & viewPort) override;
    const chip::app::Clusters::Globals::Structs::ViewportStruct::Type & GetViewport() override { return mViewport; }

    /**
     * Sets the Viewport for a specific stream. The implementation of this HAL API is responsible
     * for updating the stream identified with the provided viewport. The invoker of this
     * API shall have already ensured that the provided viewport conforms to the specification
     * requirements on size and aspect ratio.
     *
     * @param stream   the currently allocated video stream on which the viewport is being set
     * @param viewport the viewport to be set on the stream
     */
    CameraError SetViewport(VideoStream & stream,
                            const chip::app::Clusters::Globals::Structs::ViewportStruct::Type & viewport) override;

    // Get/Set SoftRecordingPrivacyMode.
    CameraError SetSoftRecordingPrivacyModeEnabled(bool softRecordingPrivacyMode) override;
    bool GetSoftRecordingPrivacyModeEnabled() override { return mSoftRecordingPrivacyModeEnabled; }

    // Get/Set SoftLivestreamPrivacyMode.
    CameraError SetSoftLivestreamPrivacyModeEnabled(bool softLivestreamPrivacyMode) override;
    bool GetSoftLivestreamPrivacyModeEnabled() override { return mSoftLivestreamPrivacyModeEnabled; }

    // Currently, defaulting to supporting hard privacy switch.
    bool HasHardPrivacySwitch() override { return true; }

    // Currently, defaulting to supporting speaker.
    bool HasSpeaker() override { return true; }

    // Mute/Unmute speaker.
    CameraError SetSpeakerMuted(bool muteSpeaker) override;
    bool GetSpeakerMuted() override { return mSpeakerMuted; }

    // Get/Set speaker volume level.
    CameraError SetSpeakerVolume(uint8_t speakerVol) override;
    uint8_t GetSpeakerVolume() override { return mSpeakerVol; }

    // Get the speaker max and min levels.
    uint8_t GetSpeakerMaxLevel() override { return mSpeakerMaxLevel; }
    uint8_t GetSpeakerMinLevel() override { return mSpeakerMinLevel; }

    // Does camera have a microphone
    bool HasMicrophone() override { return true; }

    // Mute/Unmute microphone.
    CameraError SetMicrophoneMuted(bool muteMicrophone) override;
    bool GetMicrophoneMuted() override { return mMicrophoneMuted; }

    // Set microphone volume level.
    CameraError SetMicrophoneVolume(uint8_t microphoneVol) override;
    uint8_t GetMicrophoneVolume() override { return mMicrophoneVol; }

    // Get the microphone max and min levels.
    uint8_t GetMicrophoneMaxLevel() override { return mMicrophoneMaxLevel; }
    uint8_t GetMicrophoneMinLevel() override { return mMicrophoneMinLevel; }

    // Get/Set image control attributes
    CameraError SetImageRotation(uint16_t imageRotation) override;
    uint16_t GetImageRotation() override { return mImageRotation; }

    CameraError SetImageFlipHorizontal(bool imageFlipHorizontal) override;
    bool GetImageFlipHorizontal() override { return mImageFlipHorizontal; }

    CameraError SetImageFlipVertical(bool imageFlipVertical) override;
    bool GetImageFlipVertical() override { return mImageFlipVertical; }

    // Does camera have local storage
    bool HasLocalStorage() override { return true; }

    // Set/Get LocalVideoRecordingEnabled
    CameraError SetLocalVideoRecordingEnabled(bool localVideoRecordingEnabled) override;
    bool GetLocalVideoRecordingEnabled() override { return mLocalVideoRecordingEnabled; }

    // Set/Get LocalSnapshotRecordingEnabled
    CameraError SetLocalSnapshotRecordingEnabled(bool localSnapshotRecordingEnabled) override;
    bool GetLocalSnapshotRecordingEnabled() override { return mLocalSnapshotRecordingEnabled; }

    // Does camera have a status light
    bool HasStatusLight() override { return true; }

    // Set/Get StatusLightEnabled
    CameraError SetStatusLightEnabled(bool statusLightEnabled) override;
    bool GetStatusLightEnabled() override { return mStatusLightEnabled; }

    int16_t GetPanMin() override;

    int16_t GetPanMax() override;

    int16_t GetTiltMin() override;

    int16_t GetTiltMax() override;

    uint8_t GetZoomMax() override;

    uint8_t GetMaxZones() override { return kMaxZones; }

    uint8_t GetMaxUserDefinedZones() override { return kMaxUserDefinedZones; }

    uint8_t GetSensitivityMax() override { return kSensitivityMax; }

    uint8_t GetDetectionSensitivity() override { return mDetectionSensitivity; }

    CameraError SetDetectionSensitivity(uint8_t aSensitivity) override;

    CameraError CreateZoneTrigger(const chip::app::Clusters::ZoneManagement::ZoneTriggerControlStruct & zoneTrigger) override;

    CameraError UpdateZoneTrigger(const chip::app::Clusters::ZoneManagement::ZoneTriggerControlStruct & zoneTrigger) override;

    CameraError RemoveZoneTrigger(uint16_t zoneID) override;

    CameraError SetPan(int16_t aPan) override;
    CameraError SetTilt(int16_t aTilt) override;
    CameraError SetZoom(uint8_t aZoom) override;

    std::vector<VideoStream> & GetAvailableVideoStreams() override { return mVideoStreams; }

    std::vector<AudioStream> & GetAvailableAudioStreams() override { return mAudioStreams; }

    std::vector<SnapshotStream> & GetAvailableSnapshotStreams() override { return mSnapshotStreams; }

    void SetVideoDevicePath(const std::string & path) { mVideoDevicePath = path; }

    void HandleSimulatedZoneTriggeredEvent(uint16_t zoneID);

    void HandleSimulatedZoneStoppedEvent(uint16_t zoneID);

private:
    int videoDeviceFd            = -1;
    std::string mVideoDevicePath = kDefaultVideoDevicePath;
    std::vector<VideoStream> mVideoStreams;       // Vector to hold available video streams
    std::vector<AudioStream> mAudioStreams;       // Vector to hold available audio streams
    std::vector<SnapshotStream> mSnapshotStreams; // Vector to hold available snapshot streams

    void InitializeVideoStreams();
    void InitializeAudioStreams();
    void InitializeSnapshotStreams();

    GstElement * CreateVideoPipeline(const std::string & device, int width, int height, int framerate, CameraError & error);
    GstElement * CreateAudioPipeline(const std::string & device, int channels, int sampleRate, CameraError & error);
    GstElement * CreateSnapshotPipeline(const std::string & device, int width, int height, int quality, int frameRate,
                                        const std::string & filename, CameraError & error);
    CameraError SetV4l2Control(uint32_t controlId, int value);

    // Various cluster server delegates
    chip::app::Clusters::ChimeManager mChimeManager;
    chip::app::Clusters::WebRTCTransportProvider::WebRTCProviderManager mWebRTCProviderManager;
    chip::app::Clusters::CameraAvStreamManagement::CameraAVStreamManager mCameraAVStreamManager;
    chip::app::Clusters::PushAvStreamTransport::PushAvStreamTransportManager mPushAVTransportManager;
    chip::app::Clusters::CameraAvSettingsUserLevelManagement::CameraAVSettingsUserLevelManager mCameraAVSettingsUserLevelManager;
    chip::app::Clusters::ZoneManagement::ZoneManager mZoneManager;

    DefaultMediaController mMediaController;

    int16_t mPan  = chip::app::Clusters::CameraAvSettingsUserLevelManagement::kDefaultPan;
    int16_t mTilt = chip::app::Clusters::CameraAvSettingsUserLevelManagement::kDefaultTilt;
    uint8_t mZoom = chip::app::Clusters::CameraAvSettingsUserLevelManagement::kDefaultZoom;
    // Use a standard 1080p aspect ratio
    chip::app::Clusters::Globals::Structs::ViewportStruct::Type mViewport = { 0, 0, 1920, 1080 };
    uint16_t mCurrentVideoFrameRate                                       = kMinVideoFrameRate;
    bool mHDREnabled                                                      = false;
    bool mSpeakerMuted                                                    = false;
    bool mMicrophoneMuted                                                 = false;
    bool mHardPrivacyModeOn                                               = false;
    chip::app::Clusters::CameraAvStreamManagement::TriStateAutoEnum mNightVision =
        chip::app::Clusters::CameraAvStreamManagement::TriStateAutoEnum::kOff;
    bool mSoftRecordingPrivacyModeEnabled  = false;
    bool mSoftLivestreamPrivacyModeEnabled = false;
    uint8_t mSpeakerVol                    = kSpeakerMinLevel;
    uint8_t mSpeakerMinLevel               = kSpeakerMinLevel;
    uint8_t mSpeakerMaxLevel               = kSpeakerMaxLevel;
    uint8_t mMicrophoneVol                 = kMicrophoneMinLevel;
    uint8_t mMicrophoneMinLevel            = kMicrophoneMinLevel;
    uint8_t mMicrophoneMaxLevel            = kMicrophoneMaxLevel;
    bool mLocalVideoRecordingEnabled       = false;
    bool mLocalSnapshotRecordingEnabled    = false;
    bool mStatusLightEnabled               = false;
    uint16_t mImageRotation                = kMinImageRotation;
    bool mImageFlipHorizontal              = false;
    bool mImageFlipVertical                = false;
    uint8_t mDetectionSensitivity          = (1 + kSensitivityMax) / 2; // Average over the range

    std::vector<StreamUsageEnum> mStreamUsagePriorities = { StreamUsageEnum::kLiveView, StreamUsageEnum::kRecording };
};

} // namespace Camera
