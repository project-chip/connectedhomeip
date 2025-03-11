#pragma once

#include <app/clusters/camera-av-stream-management-server/camera-av-stream-management-server.h>
#include <string>
#include <vector>


namespace CameraHAL {

using chip::app::Clusters::CameraAvStreamManagement::VideoStreamStruct;
using chip::app::Clusters::CameraAvStreamManagement::AudioStreamStruct;
using chip::app::Clusters::CameraAvStreamManagement::SnapshotStreamStruct;
using chip::app::Clusters::CameraAvStreamManagement::VideoResolutionStruct;
using chip::app::Clusters::CameraAvStreamManagement::VideoSensorParamsStruct;
using chip::app::Clusters::CameraAvStreamManagement::ImageSnapshot;

  // Enumeration for common camera errors
  enum class CameraError
  {
    SUCCESS,
    ERROR_INIT_FAILED,
    ERROR_VIDEO_STREAM_START_FAILED,
    ERROR_VIDEO_STREAM_STOP_FAILED,
    ERROR_AUDIO_STREAM_START_FAILED,
    ERROR_AUDIO_STREAM_STOP_FAILED,
    ERROR_CAPTURE_SNAPSHOT_FAILED,
    ERROR_CONFIG_FAILED,
    ERROR_NOT_IMPLEMENTED, // For features not supported on a platform
  };

  // Class defining the Camera HAL interface
  class CameraHALInterface {
  public:
    // Virtual destructor
    virtual ~CameraHALInterface() = default;

    // Initialize the camera hardware
    virtual CameraError Initialize() = 0;

    // Configure camera settings (e.g., exposure, focus)
    virtual CameraError Configure(const std::string & setting, const std::string & value) = 0;

    virtual std::optional<uint16_t> AllocateVideoStream(const VideoStreamStruct & videoStreamParams) = 0;
    virtual std::optional<uint16_t> AllocateAudioStream(const AudioStreamStruct & videoStreamParams) = 0;
    virtual std::optional<uint16_t> AllocateSnapshotStream(const SnapshotStreamStruct & videoStreamParams) = 0;

    virtual CameraError DeallocateVideoStream(uint16_t streamID) = 0;
    virtual CameraError DeallocateAudioStream(uint16_t streamID) = 0;
    virtual CameraError DeallocateSnapshotStream(uint16_t streamID) = 0;

    // Capture a snapshot image
    virtual CameraError CaptureSnapshot(uint16_t streamID, const VideoResolutionStruct & resolution,
                                        ImageSnapshot & outImageSnapshot) = 0;
    // Start video stream
    virtual CameraError StartVideoStream(const VideoStreamStruct & videoStreamParams) = 0;

    // Stop video stream
    virtual CameraError StopVideoStream(uint16_t streamID) = 0;

    // Start audio stream
    virtual CameraError StartAudioStream(const AudioStreamStruct & params) = 0;

    // Stop audio stream
    virtual CameraError StopAudioStream(uint16_t streamID) = 0;

    virtual VideoSensorParamsStruct & GetVideoSensorParams() = 0;

    virtual bool GetNightVisionCapable() = 0;

    virtual VideoResolutionStruct & GetMinViewport() = 0;

    virtual uint8_t GetMaxConcurrentVideoEncoders() = 0;

    virtual uint32_t GetMaxEncodedPixelRate() = 0;

    virtual uint16_t GetFrameRate() = 0;

    virtual void SetHDRMode(bool hdrMode) = 0;

  };

} // namespace CameraHAL