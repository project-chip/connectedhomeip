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
#include <fcntl.h> // For file descriptor operations
#include <fstream>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>
#include <linux/videodev2.h> // For V4L2 definitions
#include <sys/ioctl.h>

// File used to store snapshot from stream and return for CaptureSnapshot
// command.
#define SNAPSHOT_FILE_PATH "./capture_snapshot.jpg"

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Chime;
using namespace chip::app::Clusters::CameraAvStreamManagement;
using namespace Camera;

CameraDevice::CameraDevice()
{
    InitializeCameraDevice();

    InitializeStreams();

    // Initialize Video Sources
    mNetworkVideoSource.Init(&mMediaController, VIDEO_STREAM_GST_DEST_PORT, StreamType::kVideo);

    // Initialize Audio Sources
    mNetworkVideoSource.Init(&mMediaController, AUDIO_STREAM_GST_DEST_PORT, StreamType::kAudio);
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

    // TODO: Replace hardcoded device file with something passed in from
    // camera-app.
    videoDeviceFd = open("/dev/video0", O_RDWR);
    if (videoDeviceFd == -1)
    {
        ChipLogError(Zcl, "Error opening video device: %s", strerror(errno));
        return CameraError::ERROR_INIT_FAILED;
    }

    return CameraError::SUCCESS;
}

CameraError CameraDevice::InitializeStreams()
{
    InitializeVideoStreams();
    InitializeAudioStreams();
    InitializeSnapshotStreams();

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

            if (stream.codec == allocateArgs.videoCodec && stream.frameRate >= allocateArgs.minFrameRate
                && stream.frameRate <= allocateArgs.maxFrameRate && stream.videoRes.width >= allocateArgs.minResolution.width
                && stream.videoRes.height >= allocateArgs.minResolution.height && stream.videoRes.width <= allocateArgs.maxResolution.width
                && stream.videoRes.height <= allocateArgs.maxResolution.height)
            {
                stream.isAllocated = true;
                outStreamID        = stream.id;

                // Start the video stream from HAL for serving.
                StartVideoStream(stream.id);

                // Start the network stream source
                mNetworkVideoSource.Start(stream.id);

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
            // Stop the video stream
            StopVideoStream(stream.id);

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

                // Start the snapshot stream for serving.
                StartSnapshotStream(stream.id);

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
            // Stop the snapshot stream for serving.
            StopSnapshotStream(stream.id);
            stream.isAllocated = false;
            break;
        }
    }

    return CameraError::SUCCESS;
}

// Function to create the GStreamer pipeline
GstElement * CameraDevice::CreateSnapshotPipeline(const std::string & device, int width, int height, int quality,
                                                  const std::string & filename, CameraError & error)
{
    GstElement *pipeline, *source, *jpeg_caps, *videorate, *videorate_caps, *queue, *filesink;

    // Create the pipeline elements
    pipeline = gst_pipeline_new("snapshot-pipeline");
    // TODO: Have the video source passed in.
    source         = gst_element_factory_make("v4l2src", "source");
    jpeg_caps      = gst_element_factory_make("capsfilter", "jpeg_caps");
    videorate      = gst_element_factory_make("videorate", "videorate");
    videorate_caps = gst_element_factory_make("capsfilter", "timelapse_framerate");
    queue          = gst_element_factory_make("queue", "queue");
    filesink       = gst_element_factory_make("multifilesink", "sink");

    if (!pipeline || !source || !jpeg_caps || !videorate || !videorate_caps || !queue || !filesink)
    {
        ChipLogError(Zcl, "Not all elements could be created.");
        gst_object_unref(pipeline);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    // Add elements to the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, jpeg_caps, videorate, videorate_caps, queue, filesink, NULL);

    // Link the elements
    if (gst_element_link_many(source, jpeg_caps, videorate, videorate_caps, queue, filesink, NULL) != TRUE)
    {
        ChipLogError(Zcl, "Elements could not be linked.");
        gst_object_unref(pipeline);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    // Set the source device and caps
    g_object_set(source, "device", device.c_str(), "do-timestamp", TRUE, NULL);

    GstCaps * caps = gst_caps_new_simple("image/jpeg", "width", G_TYPE_INT, width, "height", G_TYPE_INT, height, "framerate",
                                         GST_TYPE_FRACTION, 30, 1, "quality", G_TYPE_INT, quality, NULL);
    g_object_set(jpeg_caps, "caps", caps, NULL);
    gst_caps_unref(caps);

    // Set the output file location
    g_object_set(filesink, "location", filename.c_str(), NULL);

    return pipeline;
}

// Helper function to create a GStreamer pipeline
GstElement * CameraDevice::CreateVideoPipeline(const std::string & pipelineString, CameraError & error)
{
    GError * gerror       = nullptr;
    GstElement * pipeline = gst_parse_launch(pipelineString.c_str(), &gerror);
    if (gerror != nullptr)
    {
        ChipLogError(Zcl, "Error creating pipeline: %s", gerror->message);
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
    control.id    = controlId;
    control.value = value;

    if (ioctl(videoDeviceFd, VIDIOC_S_CTRL, &control) == -1)
    {
        ChipLogError(Zcl, "Error setting V4L2 control: %s", strerror(errno));

        return CameraError::ERROR_CONFIG_FAILED;
    }

    return CameraError::SUCCESS;
}

CameraError CameraDevice::CaptureSnapshot(const uint16_t streamID, const VideoResolutionStruct & resolution,
                                          ImageSnapshot & outImageSnapshot)
{
    // Read from image file stored from snapshot stream.
    std::ifstream file(SNAPSHOT_FILE_PATH, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        ChipLogError(Zcl, "Error opening snapshot image file: ");
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Ensure space for image snapshot data in outImageSnapshot
    outImageSnapshot.data.resize(static_cast<size_t>(size));

    if (!file.read(reinterpret_cast<char *>(outImageSnapshot.data.data()), size))
    {
        ChipLogError(Zcl, "Error reading image file: ");
        file.close();
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    file.close();

    auto it = std::find_if(snapshotStreams.begin(), snapshotStreams.end(),
                           [streamID](const SnapshotStream & s) { return s.id == streamID; });
    if (it == snapshotStreams.end())
    {
        ChipLogError(Zcl, "Snapshot streamID : %u not found", streamID);
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    outImageSnapshot.imageRes.width  = it->videoRes.width;
    outImageSnapshot.imageRes.height = it->videoRes.height;
    outImageSnapshot.imageCodec      = it->codec;

    return CameraError::SUCCESS;
}

CameraError CameraDevice::StartVideoStream(uint16_t streamID)
{
    auto it =
        std::find_if(videoStreams.begin(), videoStreams.end(), [streamID](const VideoStream & s) { return s.id == streamID; });

    if (it == videoStreams.end())
    {
        return CameraError::ERROR_VIDEO_STREAM_START_FAILED;
    }

    // TODO: Replace with Gst API based pipeline creation
    // Construct RTP video pipeline
    std::string pipelineString = "v4l2src device=/dev/video0 ! "
                                 "video/x-raw,width=" +
        std::to_string(it->videoRes.width) + ",height=" + std::to_string(it->videoRes.height) +
        ",framerate=" + std::to_string(it->frameRate) + "/1 ! ";

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

    pipelineString +=
        "udpsink host=" + std::string(STREAM_GST_DEST_IP) + " port=" + std::to_string(VIDEO_STREAM_GST_DEST_PORT); // Known socket

    CameraError error = CameraError::SUCCESS;
    it->videoPipeline = CreateVideoPipeline(pipelineString, error);
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
    auto it =
        std::find_if(videoStreams.begin(), videoStreams.end(), [streamID](const VideoStream & s) { return s.id == streamID; });

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
    auto it = std::find_if(snapshotStreams.begin(), snapshotStreams.end(),
                           [streamID](const SnapshotStream & s) { return s.id == streamID; });
    if (it == snapshotStreams.end())
    {
        ChipLogError(Zcl, "Snapshot streamID : %u not found", streamID);
        return CameraError::ERROR_SNAPSHOT_STREAM_START_FAILED;
    }

    // Create the GStreamer pipeline
    CameraError error = CameraError::SUCCESS;
    it->snapshotPipeline =
        CreateSnapshotPipeline("/dev/video0", it->videoRes.width, it->videoRes.height, it->quality, "capture_snapshot.jpg", error);
    if (it->snapshotPipeline == nullptr)
    {
        return error;
    }

    // Start the pipeline
    GstStateChangeReturn result = gst_element_set_state(it->snapshotPipeline, GST_STATE_PLAYING);
    if (result == GST_STATE_CHANGE_FAILURE)
    {
        ChipLogError(Zcl, "Failed to start snapshot pipeline.");
        gst_object_unref(it->snapshotPipeline);
        it->snapshotPipeline = nullptr;
        return CameraError::ERROR_SNAPSHOT_STREAM_START_FAILED;
    }

    // Wait for the pipeline to reach the PLAYING state
    GstState state;
    gst_element_get_state(it->snapshotPipeline, &state, nullptr, GST_CLOCK_TIME_NONE);
    if (state != GST_STATE_PLAYING)
    {
        ChipLogError(Zcl, "Snapshot pipeline did not reach PLAYING state.");
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
    auto it = std::find_if(snapshotStreams.begin(), snapshotStreams.end(),
                           [streamID](const SnapshotStream & s) { return s.id == streamID; });
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
    static VideoSensorParamsStruct videoSensorParams = { 4608, 2592, 120,
                                                         chip::Optional<uint16_t>(30) }; // Typical numbers for Pi camera.
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

void CameraDevice::SetHDRMode(bool hdrMode) {}

void CameraDevice::InitializeVideoStreams()
{
    // Allocate single video stream with typical supported parameters
    VideoStream videoStream = { 1, false, VideoCodecEnum::kH264, { 640, 480 }, 30, nullptr };

    videoStreams.push_back(videoStream);
}

void CameraDevice::InitializeAudioStreams()
{
    // Allocate single audio stream with typical supported parameters
    AudioStream audioStream = { 1, false, AudioCodecEnum::kOpus, 2, nullptr };

    audioStreams.push_back(audioStream);
}

void CameraDevice::InitializeSnapshotStreams()
{
    // Allocate single snapshot stream with typical supported parameters
    snapshotStreams.push_back({ 1, false, ImageCodecEnum::kJpeg, { 320, 240 }, 90, nullptr });
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
