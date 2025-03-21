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
#include <AppMain.h>
#include <iostream>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <sys/ioctl.h>
#include <fcntl.h> // For file descriptor operations
#include <linux/videodev2.h> // For V4L2 definitions
#include <lib/support/logging/CHIPLogging.h>
//#include <gst/app/gstappsrc.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Chime;
using namespace chip::app::Clusters::CameraAvStreamManagement;
using namespace Camera;

CameraDevice::CameraDevice()
{
    InitializeCameraDevice();

    InitializeStreams();
}

CameraDevice::~CameraDevice()
{
    if (videoDeviceFd != -1)
    {
        close(videoDeviceFd);
    }
}

CameraError CameraDevice::InitializeCameraDevice()
{
    static bool gstreamerInitialized = false;

    if (!gstreamerInitialized)
    {
        gst_init(nullptr, nullptr);
        gstreamerInitialized = true;
    }

    videoDeviceFd = open("/dev/video0", O_RDWR);
    if (videoDeviceFd == -1)
    {
        ChipLogError(NotSpecified, "Error opening video device: %s", strerror(errno));
        return CameraError::ERROR_INIT_FAILED;
    }

    // TODO: Check capabilities of the camera

    return CameraError::SUCCESS;
}

CameraError CameraDevice::InitializeStreams()
{
    InitializeVideoStreams();
    InitializeAudioStreams();
    InitializeSnapshotStreams();

    StartVideoStream(1);
    StartSnapshotStream(1);
    return CameraError::SUCCESS;
}

CameraError CameraDevice::VideoStreamAllocate(const VideoStreamStruct & allocateArgs, uint16_t & outStreamID)
{
    outStreamID               = kInvalidStreamID;
    bool foundAvailableStream = false;

    for (VideoStream & stream : videoStreams)
    {
        if (!stream.isAllocated)
        {
            foundAvailableStream = true;

            if (stream.codec == allocateArgs.videoCodec)
            {
                stream.isAllocated = true;
                outStreamID        = stream.id;
                return CameraError::SUCCESS;
            }
        }
    }

    if (!foundAvailableStream)
    {
        return CameraError::ERROR_RESOURCE_EXHAUSTED;
    }

    return CameraError::ERROR_VIDEO_STREAM_ALLOC_FAILED;
}

CameraError CameraDevice::VideoStreamDeallocate(const uint16_t streamID)
{
    for (VideoStream & stream : videoStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            stream.isAllocated = false;
            break;
        }
    }

    return CameraError::SUCCESS;
}

CameraError CameraDevice::AudioStreamAllocate(const AudioStreamStruct & allocateArgs, uint16_t & outStreamID)
{
    outStreamID = kInvalidStreamID;

    bool foundAvailableStream = false;

    for (AudioStream & stream : audioStreams)
    {
        if (!stream.isAllocated)
        {
            foundAvailableStream = true;

            if (stream.codec == allocateArgs.audioCodec)
            {
                stream.isAllocated = true;
                outStreamID        = stream.id;
                return CameraError::SUCCESS;
            }
        }
    }

    if (!foundAvailableStream)
    {
        return CameraError::ERROR_RESOURCE_EXHAUSTED;
    }

    return CameraError::ERROR_AUDIO_STREAM_ALLOC_FAILED;
}

CameraError CameraDevice::AudioStreamDeallocate(const uint16_t streamID)
{
    for (AudioStream & stream : audioStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            stream.isAllocated = false;
            break;
        }
    }

    return CameraError::SUCCESS;
}

CameraError CameraDevice::SnapshotStreamAllocate(const SnapshotStreamStruct & allocateArgs, uint16_t & outStreamID)
{
    outStreamID = kInvalidStreamID;

    bool foundAvailableStream = false;

    for (SnapshotStream & stream : snapshotStreams)
    {
        if (!stream.isAllocated)
        {
            foundAvailableStream = true;

            if (stream.codec == allocateArgs.imageCodec)
            {
                stream.isAllocated = true;
                outStreamID        = stream.id;
                return CameraError::SUCCESS;
            }
        }
    }

    if (!foundAvailableStream)
    {
        return CameraError::ERROR_RESOURCE_EXHAUSTED;
    }

    return CameraError::ERROR_SNAPSHOT_STREAM_ALLOC_FAILED;
}

CameraError CameraDevice::SnapshotStreamDeallocate(const uint16_t streamID)
{
    for (SnapshotStream & stream : snapshotStreams)
    {
        if (stream.id == streamID && stream.isAllocated)
        {
            stream.isAllocated = false;
            break;
        }
    }

    return CameraError::SUCCESS;
}

// Helper function to create a GStreamer pipeline
GstElement* CameraDevice::CreatePipeline(const std::string & pipelineString, CameraError& error)
{
    GError * gerror = nullptr;
    GstElement * pipeline = gst_parse_launch(pipelineString.c_str(), &gerror);
    if (gerror != nullptr) {
        ChipLogError(NotSpecified, "Error creating pipeline: %s", gerror->message);
        g_error_free(gerror);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }
    return pipeline;
}

// Helper function to set V4L2 control
CameraError CameraDevice::SetV4l2Control(uint32_t controlId, int value)
{
    if (videoDeviceFd == -1)
    {
        return CameraError::ERROR_INIT_FAILED;
    }

    v4l2_control control;
    control.id = controlId;
    control.value = value;

    if (ioctl(videoDeviceFd, VIDIOC_S_CTRL, &control) == -1)
    {
        ChipLogError(NotSpecified, "Error setting V4L2 control: %s", strerror(errno));

        return CameraError::ERROR_CONFIG_FAILED;
    }

    return CameraError::SUCCESS;
}

CameraError CameraDevice::CaptureSnapshot(const uint16_t streamID, const VideoResolutionStruct & resolution,
                            ImageSnapshot & outImageSnapshot)
{
    return CameraError::ERROR_NOT_IMPLEMENTED;
}

CameraError CameraDevice::StartVideoStream(uint16_t streamID)
{
    auto it = std::find_if(videoStreams.begin(), videoStreams.end(),
                           [streamID](const VideoStream & s) { return s.id == streamID; });

    if (it == videoStreams.end())
    {
        return CameraError::ERROR_VIDEO_STREAM_START_FAILED;
    }

    // Construct RTP video pipeline
    std::string pipelineString = "v4l2src device=/dev/video0 ! "
                                 "video/x-raw,width=" + std::to_string(it->videoRes.width) +
                                 ",height=" + std::to_string(it->videoRes.height) +
                                 ",framerate=" + std::to_string(it->frameRate) +
                                 "/1 ! ";

    if (it->codec == VideoCodecEnum::kH264)
    {
        pipelineString += "videoconvert ! videoscale ! x264enc tune=zerolatency ! rtph264pay ! ";
    }
    else if (it->codec == VideoCodecEnum::kHevc)
    {
        pipelineString += ""; // TODO
    }
    else
    {
        return CameraError::ERROR_VIDEO_STREAM_START_FAILED;
    }

    //pipelineString += "udpsink host=127.0.0.1 port=" + VIDEO_STREAM_GST_DEST_PORT; // Known socket
    pipelineString += "udpsink host=127.0.0.1 port=5000"; // Known socket

    CameraError error = CameraError::SUCCESS;
    it->videoPipeline = CreatePipeline(pipelineString, error);
    if (it->videoPipeline == nullptr)
    {
        return error;
    }

    // Start the pipeline
    gst_element_set_state(it->videoPipeline, GST_STATE_PLAYING);

    return CameraError::SUCCESS;
}

// Stop video stream
CameraError CameraDevice::StopVideoStream(uint16_t streamID)
{
    auto it = std::find_if(videoStreams.begin(), videoStreams.end(),
                           [streamID](const VideoStream & s) { return s.id == streamID; });

    if (it == videoStreams.end())
    {
        return CameraError::ERROR_VIDEO_STREAM_STOP_FAILED;
    }

    if (it->videoPipeline != nullptr)
    {
        gst_element_set_state(it->videoPipeline, GST_STATE_NULL);
        gst_object_unref(it->videoPipeline);
        it->videoPipeline = nullptr;
    }

    return CameraError::SUCCESS;
}

// Start audio stream
CameraError CameraDevice::StartAudioStream(uint16_t streamID)
{
    return CameraError::ERROR_NOT_IMPLEMENTED;
}

// Stop audio stream
CameraError CameraDevice::StopAudioStream(uint16_t streamID)
{
    return CameraError::ERROR_NOT_IMPLEMENTED;
}

// Start snapshot stream
CameraError CameraDevice::StartSnapshotStream(uint16_t streamID)
{
    auto it = std::find_if(snapshotStreams.begin(), snapshotStreams.end(), [streamID](const SnapshotStream & s) { return s.id == streamID; });
    if (it == snapshotStreams.end())
    {
        return CameraError::ERROR_SNAPSHOT_STREAM_START_FAILED;
    }

#if 0
    // Construct the GStreamer pipeline string
    std::string pipelineString = "v4l2src device=/dev/video0 ! "
                                 //"video/x-raw,width=" + std::to_string(it->videoRes.width) +
                                 "image/jpeg,width=" + std::to_string(it->videoRes.width) +
                                 ",height=" + std::to_string(it->videoRes.height) + ",framerate=1/1 ! ";

    if (it->codec == ImageCodecEnum::kJpeg)
    {
        pipelineString += "jpegenc quality=" + std::to_string(it->quality) + " ! ";
    }
    else
    {
        return CameraError::ERROR_SNAPSHOT_STREAM_START_FAILED;
    }
#endif

    std::string pipelineString = "v4l2src device=/dev/video0 ! videoconvert ! jpegenc snapshot=true ! ";
    pipelineString += "filesink location=./capture_snapshot.jpg";

    // Create the GStreamer pipeline
    CameraError error = CameraError::SUCCESS;
    it->snapshotPipeline = CreatePipeline(pipelineString, error);
    if (it->snapshotPipeline == nullptr)
    {
        return error;
    }

    // Start the pipeline
    GstStateChangeReturn result = gst_element_set_state(it->snapshotPipeline, GST_STATE_PLAYING);
    if (result == GST_STATE_CHANGE_FAILURE)
    {
        ChipLogError(NotSpecified, "Failed to start snapshot pipeline.");
        gst_object_unref(it->snapshotPipeline);
        it->snapshotPipeline = nullptr;
        return CameraError::ERROR_SNAPSHOT_STREAM_START_FAILED;
    }

    // Wait for the pipeline to reach the PLAYING state
    GstState state;
    gst_element_get_state(it->snapshotPipeline, &state, nullptr, GST_CLOCK_TIME_NONE);
    if (state != GST_STATE_PLAYING)
    {
        ChipLogError(NotSpecified, "Snapshot pipeline did not reach PLAYING state.");
        gst_element_set_state(it->snapshotPipeline, GST_STATE_NULL);
        gst_object_unref(it->snapshotPipeline);
        it->snapshotPipeline = nullptr;
        return CameraError::ERROR_SNAPSHOT_STREAM_START_FAILED;
    }

    return CameraError::SUCCESS;
}

// Stop snapshot stream
CameraError CameraDevice::StopSnapshotStream(uint16_t streamID)
{
    auto it = std::find_if(snapshotStreams.begin(), snapshotStreams.end(), [streamID](const SnapshotStream & s) { return s.id == streamID; });
    if (it == snapshotStreams.end())
    {
        return CameraError::ERROR_SNAPSHOT_STREAM_STOP_FAILED;
    }

    if (it->snapshotPipeline != nullptr)
    {
        // Stop the pipeline
        GstStateChangeReturn result = gst_element_set_state(it->snapshotPipeline, GST_STATE_NULL);
        if (result == GST_STATE_CHANGE_FAILURE)
        {
            return CameraError::ERROR_SNAPSHOT_STREAM_STOP_FAILED;
        }

        // Unreference the pipeline
        gst_object_unref(it->snapshotPipeline);
        it->snapshotPipeline = nullptr;
    }

    return CameraError::SUCCESS;
}

VideoSensorParamsStruct & CameraDevice::GetVideoSensorParams()
{
    static VideoSensorParamsStruct videoSensorParams = { 4608, 2592, 120, chip::Optional<uint16_t>(30) }; // Typical numbers for Pi camera.
    return videoSensorParams;
}

bool CameraDevice::GetNightVisionCapable()
{
    return false;
}

VideoResolutionStruct & CameraDevice::GetMinViewport()
{
    static VideoResolutionStruct minViewport = { 854, 480 }; // Assuming 480p resolution.
    return minViewport;
}

uint8_t CameraDevice::GetMaxConcurrentVideoEncoders()
{
    return 1;
}

uint32_t CameraDevice::GetMaxEncodedPixelRate()
{
    return 10000;
}

uint16_t CameraDevice::GetFrameRate()
{
    return 60;
}

void CameraDevice::SetHDRMode(bool hdrMode)
{

}

void CameraDevice::InitializeVideoStreams()
{
    VideoStream videoStream = {1, false, VideoCodecEnum::kH264, {640, 480}, 30, nullptr};

    videoStreams.push_back(videoStream);
}

void CameraDevice::InitializeAudioStreams()
{
    AudioStream audioStream = {1, false, AudioCodecEnum::kOpus, 2, nullptr};

    audioStreams.push_back(audioStream);
}

void CameraDevice::InitializeSnapshotStreams()
{
    snapshotStreams.push_back({ 1, false, ImageCodecEnum::kJpeg, {168, 112}, 90, nullptr });
}

ChimeDelegate & CameraDevice::GetChimeDelegate()
{
    return mChimeManager;
}

WebRTCTransportProvider::Delegate & CameraDevice::GetWebRTCProviderDelegate()
{
    return mWebRTCProviderManager;
}

CameraAVStreamMgmtDelegate & CameraDevice::GetCameraAVStreamMgmtDelegate()
{
    return mCameraAVStreamManager;
}
