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
#include <app/clusters/camera-av-settings-user-level-management-server/camera-av-settings-user-level-management-server.h>
#include <app/clusters/camera-av-stream-management-server/camera-av-stream-management-server.h>
#include <app/clusters/chime-server/chime-server.h>
#include <app/clusters/webrtc-transport-provider-server/webrtc-transport-provider-server.h>
#include <media-controller.h>

using chip::app::Clusters::CameraAvStreamManagement::AudioStreamStruct;
using chip::app::Clusters::CameraAvStreamManagement::ImageSnapshot;
using chip::app::Clusters::CameraAvStreamManagement::SnapshotStreamStruct;
using chip::app::Clusters::CameraAvStreamManagement::VideoResolutionStruct;
using chip::app::Clusters::CameraAvStreamManagement::VideoSensorParamsStruct;
using chip::app::Clusters::CameraAvStreamManagement::VideoStreamStruct;
using chip::app::Clusters::CameraAvStreamManagement::ViewportStruct;

struct VideoStream
{
    VideoStreamStruct videoStreamParams;
    bool isAllocated;    // Flag to indicate if the stream is allocated.
    void * videoContext; // Platform-specific context object associated with
                         // video stream;

    bool IsCompatible(const VideoStreamStruct & inputParams) const
    {
        return (videoStreamParams.videoCodec == inputParams.videoCodec &&
                videoStreamParams.minFrameRate <= inputParams.minFrameRate &&
                videoStreamParams.maxFrameRate >= inputParams.maxFrameRate &&
                videoStreamParams.minResolution.width <= inputParams.minResolution.width &&
                videoStreamParams.minResolution.height <= inputParams.minResolution.height &&
                videoStreamParams.maxResolution.width >= inputParams.maxResolution.width &&
                videoStreamParams.maxResolution.height >= inputParams.maxResolution.height);
    }
};

struct AudioStream
{
    AudioStreamStruct audioStreamParams;
    bool isAllocated;    // Flag to indicate if the stream is allocated.
    void * audioContext; // Platform-specific context object associated with
                         // video stream;

    bool IsCompatible(const AudioStreamStruct & inputParams) const
    {
        return (audioStreamParams.audioCodec == inputParams.audioCodec &&
                audioStreamParams.channelCount == inputParams.channelCount &&
                audioStreamParams.sampleRate == inputParams.sampleRate);
    }
};

struct SnapshotStream
{
    SnapshotStreamStruct snapshotStreamParams;
    bool isAllocated;       // Flag to indicate if the stream is allocated
    void * snapshotContext; // Platform-specific context object associated with
                            // snapshot stream;

    bool IsCompatible(const SnapshotStreamStruct & inputParams) const
    {
        return (snapshotStreamParams.imageCodec == inputParams.imageCodec && snapshotStreamParams.quality == inputParams.quality &&
                snapshotStreamParams.frameRate <= inputParams.frameRate &&
                snapshotStreamParams.minResolution.width <= inputParams.minResolution.width &&
                snapshotStreamParams.minResolution.height <= inputParams.minResolution.height &&
                snapshotStreamParams.maxResolution.width >= inputParams.maxResolution.width &&
                snapshotStreamParams.maxResolution.height >= inputParams.maxResolution.height);
    }
};

// Enumeration for common camera errors
enum class CameraError
{
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

// Camera Device Interface defines all the clusters that need to be implemented for a Camera Device
class CameraDeviceInterface
{
public:
    virtual ~CameraDeviceInterface() = default;

    // Getter for Chime Delegate
    virtual chip::app::Clusters::ChimeDelegate & GetChimeDelegate() = 0;

    // Getter for WebRTCProvider Delegate
    virtual chip::app::Clusters::WebRTCTransportProvider::Delegate & GetWebRTCProviderDelegate() = 0;

    // Getter for CameraAVStreamManagement Delegate
    virtual chip::app::Clusters::CameraAvStreamManagement::CameraAVStreamMgmtDelegate & GetCameraAVStreamMgmtDelegate() = 0;

    // Getter for CameraAVSettingsUserLevelManagement Delegate
    virtual chip::app::Clusters::CameraAvSettingsUserLevelManagement::Delegate & GetCameraAVSettingsUserLevelMgmtDelegate() = 0;

    // Getter for the Media Controller
    virtual MediaController & GetMediaController() = 0;

    // Class defining the Camera HAL interface
    class CameraHALInterface
    {
    public:
        // Virtual destructor
        virtual ~CameraHALInterface() = default;

        // Initialize the camera hardware
        virtual CameraError InitializeCameraDevice() = 0;

        virtual CameraError InitializeStreams() = 0;

        // Configure camera settings (e.g., exposure, focus)
        // virtual CameraError Configure(const std::string & setting, const std::string & value) = 0;

        virtual std::vector<VideoStream> & GetAvailableVideoStreams() = 0;

        virtual std::vector<AudioStream> & GetAvailableAudioStreams() = 0;

        virtual std::vector<SnapshotStream> & GetAvailableSnapshotStreams() = 0;

        // Capture a snapshot image
        virtual CameraError CaptureSnapshot(uint16_t streamID, const VideoResolutionStruct & resolution,
                                            ImageSnapshot & outImageSnapshot) = 0;
        // Start video stream
        virtual CameraError StartVideoStream(uint16_t streamID) = 0;

        // Stop video stream
        virtual CameraError StopVideoStream(uint16_t streamID) = 0;

        // Start audio stream
        virtual CameraError StartAudioStream(uint16_t streamID) = 0;

        // Stop audio stream
        virtual CameraError StopAudioStream(uint16_t streamID) = 0;

        // Start snapshot stream
        virtual CameraError StartSnapshotStream(uint16_t streamID) = 0;

        // Stop snapshot stream
        virtual CameraError StopSnapshotStream(uint16_t streamID) = 0;

        // Get the maximum number of concurrent encoders supported by camera.
        virtual uint8_t GetMaxConcurrentVideoEncoders() = 0;

        // Get the maximum data rate in encoded pixels per second that the
        // camera can produce given the hardware encoders it has.
        virtual uint32_t GetMaxEncodedPixelRate() = 0;

        // Get the Video sensor params(sensor dimensions, framerate, HDR
        // capabilities)
        virtual VideoSensorParamsStruct & GetVideoSensorParams() = 0;

        // Get indication whether camera supports night vision mode
        virtual bool GetNightVisionCapable() = 0;

        // Get indication of the min resolution(pixels) that camera allows for
        // its viewport.
        virtual VideoResolutionStruct & GetMinViewport() = 0;

        // Get the maximum size of content buffer in bytes. This buffer holds
        // compressed and/or raw audio/video content.
        virtual uint32_t GetMaxContentBufferSize() = 0;

        // Get the maximum network bandwidth(mbps) that the camera would consume
        // for transmission of its media streams.
        virtual uint32_t GetMaxNetworkBandwidth() = 0;

        // Get the current frame rate of the camera sensor.
        virtual uint16_t GetCurrentFrameRate() = 0;

        // Enable/Disable High Dynamic Range mode.
        virtual CameraError SetHDRMode(bool hdrMode) = 0;

        // Get the current camera HDR mode.
        virtual bool GetHDRMode() = 0;

        // Does camera have a speaker
        virtual bool HasSpeaker() = 0;

        // Set the viewport for all streams
        virtual CameraError SetViewport(const ViewportStruct & viewPort) = 0;

        // Get the current camera viewport.
        virtual const ViewportStruct & GetViewport() = 0;

        // Mute/Unmute speaker.
        virtual CameraError SetSpeakerMuted(bool muteSpeaker) = 0;

        // Set speaker volume level.
        virtual CameraError SetSpeakerVolume(uint8_t speakerVol) = 0;

        // Get the speaker max and min levels.
        virtual uint8_t GetSpeakerMaxLevel() = 0;
        virtual uint8_t GetSpeakerMinLevel() = 0;

        // Does camera have a microphone
        virtual bool HasMicrophone() = 0;

        // Mute/Unmute microphone.
        virtual CameraError SetMicrophoneMuted(bool muteMicrophone) = 0;
        virtual bool GetMicrophoneMuted()                           = 0;

        // Set microphone volume level.
        virtual CameraError SetMicrophoneVolume(uint8_t microphoneVol) = 0;
        virtual uint8_t GetMicrophoneVolume()                          = 0;

        // Get the microphone max and min levels.
        virtual uint8_t GetMicrophoneMaxLevel() = 0;
        virtual uint8_t GetMicrophoneMinLevel() = 0;

        virtual int16_t GetPanMin() = 0;

        virtual int16_t GetPanMax() = 0;

        virtual int16_t GetTiltMin() = 0;

        virtual int16_t GetTiltMax() = 0;

        virtual uint8_t GetZoomMax() = 0;
    };

    virtual CameraHALInterface & GetCameraHALInterface() = 0;
};
