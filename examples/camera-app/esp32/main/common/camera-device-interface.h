#pragma once
#include "camera-avstream-controller.h"
#include <app/clusters/camera-av-stream-management-server/CameraAVStreamManagementCluster.h>
#include <app/clusters/webrtc-transport-provider-server/WebRTCTransportProviderCluster.h>

using chip::app::Clusters::CameraAvStreamManagement::AudioCapabilitiesStruct;
using chip::app::Clusters::CameraAvStreamManagement::AudioStreamStruct;
using chip::app::Clusters::CameraAvStreamManagement::ImageSnapshot;
using chip::app::Clusters::CameraAvStreamManagement::
    RateDistortionTradeOffStruct;
using chip::app::Clusters::CameraAvStreamManagement::SnapshotCapabilitiesStruct;
using chip::app::Clusters::CameraAvStreamManagement::SnapshotStreamStruct;
using chip::app::Clusters::CameraAvStreamManagement::TriStateAutoEnum;
using chip::app::Clusters::CameraAvStreamManagement::VideoResolutionStruct;
using chip::app::Clusters::CameraAvStreamManagement::VideoSensorParamsStruct;
using chip::app::Clusters::CameraAvStreamManagement::VideoStreamStruct;
using chip::app::Clusters::Globals::StreamUsageEnum;

struct VideoStream {
  VideoStreamStruct videoStreamParams;
  bool isAllocated; // Flag to indicate if the stream is allocated.
  chip::app::Clusters::Globals::Structs::ViewportStruct::Type
      viewport; // Stream specific viewport, defaults to the camera viewport
  void *videoContext; // Platform-specific context object associated with
  // video stream;

  bool IsCompatible(const VideoStreamStruct &inputParams) const {
    return (videoStreamParams.videoCodec == inputParams.videoCodec &&
            videoStreamParams.minFrameRate >= inputParams.minFrameRate &&
            videoStreamParams.maxFrameRate <= inputParams.maxFrameRate &&
            videoStreamParams.minResolution.width <=
                inputParams.minResolution.width &&
            videoStreamParams.minResolution.height <=
                inputParams.minResolution.height &&
            videoStreamParams.maxResolution.width >=
                inputParams.maxResolution.width &&
            videoStreamParams.maxResolution.height >=
                inputParams.maxResolution.height &&
            videoStreamParams.minBitRate <= inputParams.minBitRate &&
            videoStreamParams.maxBitRate >= inputParams.maxBitRate &&
            videoStreamParams.keyFrameInterval == inputParams.keyFrameInterval);
  }
};

struct AudioStream {
  AudioStreamStruct audioStreamParams;
  bool isAllocated;   // Flag to indicate if the stream is allocated.
  void *audioContext; // Platform-specific context object associated with
  // video stream;

  bool IsCompatible(const AudioStreamStruct &inputParams) const {
    return (audioStreamParams.audioCodec == inputParams.audioCodec &&
            audioStreamParams.channelCount == inputParams.channelCount &&
            audioStreamParams.sampleRate == inputParams.sampleRate &&
            audioStreamParams.bitDepth == inputParams.bitDepth);
  }
};

struct SnapshotStream {
  SnapshotStreamStruct snapshotStreamParams;
  bool isAllocated;      // Flag to indicate if the stream is allocated
  void *snapshotContext; // Platform-specific context object associated with
  // snapshot stream;

  bool IsCompatible(const chip::app::Clusters::CameraAvStreamManagement::
                        CameraAVStreamManagementDelegate::SnapshotStreamAllocateArgs
                            &inputParams) const {
    return (snapshotStreamParams.imageCodec == inputParams.imageCodec &&
            snapshotStreamParams.quality == inputParams.quality &&
            snapshotStreamParams.frameRate <= inputParams.maxFrameRate &&
            snapshotStreamParams.minResolution.width <=
                inputParams.minResolution.width &&
            snapshotStreamParams.minResolution.height <=
                inputParams.minResolution.height &&
            snapshotStreamParams.maxResolution.width >=
                inputParams.maxResolution.width &&
            snapshotStreamParams.maxResolution.height >=
                inputParams.maxResolution.height);
  }
};

// Enumeration for common camera errors
enum class CameraError {
  SUCCESS,
  ERROR_INIT_FAILED,
  ERROR_VIDEO_STREAM_START_FAILED,
  ERROR_VIDEO_STREAM_STOP_FAILED,
  ERROR_AUDIO_STREAM_START_FAILED,
  ERROR_AUDIO_STREAM_STOP_FAILED,
  ERROR_SNAPSHOT_STREAM_START_FAILED,
  ERROR_SNAPSHOT_STREAM_STOP_FAILED,
  ERROR_CAPTURE_SNAPSHOT_FAILED,
  ERROR_CONFIG_FAILED,
  ERROR_RESOURCE_EXHAUSTED,
  ERROR_NOT_IMPLEMENTED, // For features not supported on a platform
};

// Camera Device Interface defines all the clusters that need to be implemented
// for a Camera Device
class CameraDeviceInterface {
public:
  virtual ~CameraDeviceInterface() = default;

  // Getter for WebRTCProvider Delegate
  virtual chip::app::Clusters::WebRTCTransportProvider::Delegate &
  GetWebRTCProviderDelegate() = 0;

    // Set the WebRTC Transport Provider server instance
    virtual void
    SetWebRTCTransportProvider(chip::app::Clusters::WebRTCTransportProvider::WebRTCTransportProviderCluster * provider) = 0;

  // Getter for CameraAVStreamManagement Delegate
  virtual chip::app::Clusters::CameraAvStreamManagement::
      CameraAVStreamManagementDelegate &
      GetCameraAVStreamMgmtDelegate() = 0;

  // Getter for CameraAVStreamManagement Controller
  virtual chip::app::Clusters::CameraAvStreamManagement::
      CameraAVStreamController &
      GetCameraAVStreamMgmtController() = 0;

  // Class defining the Camera HAL interface
  class CameraHALInterface {
  public:
    // Virtual destructor
    virtual ~CameraHALInterface() = default;

    // Initialize the camera hardware
    virtual CameraError InitializeCameraDevice() = 0;

    virtual CameraError InitializeStreams() = 0;

    virtual std::vector<VideoStream> &GetAvailableVideoStreams() = 0;

    virtual std::vector<AudioStream> &GetAvailableAudioStreams() = 0;

    virtual std::vector<SnapshotStream> &GetAvailableSnapshotStreams() = 0;

    // Capture a snapshot image
    virtual CameraError
    CaptureSnapshot(const chip::app::DataModel::Nullable<uint16_t> streamID,
                    const VideoResolutionStruct &resolution,
                    ImageSnapshot &outImageSnapshot) = 0;

    // Allocate snapshot stream
    virtual CameraError AllocateSnapshotStream(
        const chip::app::Clusters::CameraAvStreamManagement::
            CameraAVStreamManagementDelegate::SnapshotStreamAllocateArgs &args,
        uint16_t &outStreamID) = 0;

    // Get the maximum number of concurrent encoders supported by camera.
    virtual uint8_t GetMaxConcurrentEncoders() = 0;

    // Get the maximum data rate in encoded pixels per second that the
    // camera can produce given the hardware encoders it has.
    virtual uint32_t GetMaxEncodedPixelRate() = 0;

    // Get the Video sensor params(sensor dimensions, framerate, HDR
    // capabilities)
    virtual VideoSensorParamsStruct &GetVideoSensorParams() = 0;

    // Get indication whether camera supports high dynamic range for video
    virtual bool GetCameraSupportsHDR() = 0;

    // Get indication whether camera supports night vision
    virtual bool GetCameraSupportsNightVision() = 0;

    // Get indication whether camera night vision using infrared
    virtual bool GetNightVisionUsesInfrared() = 0;

    // Get indication whether camera supports image control
    virtual bool GetCameraSupportsImageControl() = 0;

    // Get indication whether camera supports watermark for video and snapshot
    virtual bool GetCameraSupportsWatermark() = 0;

    // Get indication whether camera supports on-screen display for video and
    // snapshot
    virtual bool GetCameraSupportsOSD() = 0;

    // Get indication whether camera supports soft recording and livestream
    // privacy modes
    virtual bool GetCameraSupportsSoftPrivacy() = 0;

    // Get indication of the min resolution(pixels) that camera allows for
    // its viewport.
    virtual VideoResolutionStruct &GetMinViewport() = 0;

    // Get the rate distortion tradeoff points(min bitrate for resolutions) for
    // video codecs.
    virtual std::vector<RateDistortionTradeOffStruct> &
    GetRateDistortionTradeOffPoints() = 0;

    // Get the maximum size of content buffer in bytes. This buffer holds
    // compressed and/or raw audio/video content.
    virtual uint32_t GetMaxContentBufferSize() = 0;

    // Get microphone capabilities.
    virtual AudioCapabilitiesStruct &GetMicrophoneCapabilities() = 0;

    // Get speaker capabilities.
    virtual AudioCapabilitiesStruct &GetSpeakerCapabilities() = 0;

    // Get snapshot capabilities
    virtual std::vector<SnapshotCapabilitiesStruct> &
    GetSnapshotCapabilities() = 0;

    // Get the maximum network bandwidth(mbps) that the camera would consume
    // for transmission of its media streams.
    virtual uint32_t GetMaxNetworkBandwidth() = 0;

    // Get the current frame rate of the camera sensor.
    virtual uint16_t GetCurrentFrameRate() = 0;

    // Enable/Disable High Dynamic Range mode.
    virtual CameraError SetHDRMode(bool hdrMode) = 0;

    // Get the current camera HDR mode.
    virtual bool GetHDRMode() = 0;

    // Get Supported Stream usages; Typically set by manudacturer.
    // This also sets the default priority of the stream usages.
    virtual std::vector<StreamUsageEnum> &GetSupportedStreamUsages() = 0;

    // Get stream usage priorities as an ordered list. This is expected to
    // be a subset of the SupportedStreamUsages.
    virtual std::vector<StreamUsageEnum> &GetStreamUsagePriorities() = 0;
    virtual CameraError SetStreamUsagePriorities(
        std::vector<StreamUsageEnum> streamUsagePriorities) = 0;

    // Get/Set soft recording privacy mode
    virtual CameraError
    SetSoftRecordingPrivacyModeEnabled(bool softRecordingPrivacyMode) = 0;
    virtual bool GetSoftRecordingPrivacyModeEnabled() = 0;

    // Get/Set soft livestream privacy mode
    virtual CameraError
    SetSoftLivestreamPrivacyModeEnabled(bool softLivestreamPrivacyMode) = 0;
    virtual bool GetSoftLivestreamPrivacyModeEnabled() = 0;

    // Does camera have a hard privacy switch
    virtual bool HasHardPrivacySwitch() = 0;

    // Get/Set hard privacy mode
    virtual CameraError SetHardPrivacyMode(bool hardPrivacyMode) = 0;
    virtual bool GetHardPrivacyMode() = 0;

    // Get/Set night vision
    virtual CameraError SetNightVision(TriStateAutoEnum nightVision) = 0;
    virtual TriStateAutoEnum GetNightVision() = 0;

    // Set the viewport for all streams
    virtual CameraError SetViewport(
        const chip::app::Clusters::Globals::Structs::ViewportStruct::Type
            &viewPort) = 0;

    // Get the current camera viewport.
    virtual const chip::app::Clusters::Globals::Structs::ViewportStruct::Type &
    GetViewport() = 0;

    // Set the viewport for a specific stream
    virtual CameraError SetViewport(
        VideoStream &stream,
        const chip::app::Clusters::Globals::Structs::ViewportStruct::Type
            &viewPort) = 0;

    // Does camera have a speaker
    virtual bool HasSpeaker() = 0;

    // Mute/Unmute speaker.
    virtual bool GetSpeakerMuted() = 0;
    virtual CameraError SetSpeakerMuted(bool muteSpeaker) = 0;

    // Get/Set speaker volume level.
    virtual uint8_t GetSpeakerVolume() = 0;
    virtual CameraError SetSpeakerVolume(uint8_t speakerVol) = 0;

    // Get the speaker max and min levels.
    virtual uint8_t GetSpeakerMaxLevel() = 0;
    virtual uint8_t GetSpeakerMinLevel() = 0;

    // Does camera have a microphone
    virtual bool HasMicrophone() = 0;

    // Mute/Unmute microphone.
    virtual CameraError SetMicrophoneMuted(bool muteMicrophone) = 0;
    virtual bool GetMicrophoneMuted() = 0;

    // Set microphone volume level.
    virtual CameraError SetMicrophoneVolume(uint8_t microphoneVol) = 0;
    virtual uint8_t GetMicrophoneVolume() = 0;

    // Get the microphone max and min levels.
    virtual uint8_t GetMicrophoneMaxLevel() = 0;
    virtual uint8_t GetMicrophoneMinLevel() = 0;

    // Get/Set image control attributes
    virtual CameraError SetImageRotation(uint16_t imageRotation) = 0;
    virtual uint16_t GetImageRotation() = 0;

    virtual CameraError SetImageFlipHorizontal(bool imageFlipHorizontal) = 0;
    virtual bool GetImageFlipHorizontal() = 0;

    virtual CameraError SetImageFlipVertical(bool imageFlipVertical) = 0;
    virtual bool GetImageFlipVertical() = 0;

    // Does camera have local storage
    virtual bool HasLocalStorage() = 0;

    virtual CameraError
    SetLocalVideoRecordingEnabled(bool localVideoRecordingEnabled) = 0;
    virtual bool GetLocalVideoRecordingEnabled() = 0;

    virtual CameraError
    SetLocalSnapshotRecordingEnabled(bool localSnapshotRecordingEnabled) = 0;
    virtual bool GetLocalSnapshotRecordingEnabled() = 0;

    // Does camera have a status light
    virtual bool HasStatusLight() = 0;

    virtual CameraError SetStatusLightEnabled(bool statusLightEnabled) = 0;
    virtual bool GetStatusLightEnabled();
  };

  virtual CameraHALInterface &GetCameraHALInterface() = 0;
};
