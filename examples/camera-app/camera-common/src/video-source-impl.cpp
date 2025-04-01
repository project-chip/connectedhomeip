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

#include "video-source-impl.h"

#include <gst/app/gstappsink.h>
#include <gst/gst.h>
#include <iostream>

#include <chrono>
#include <ctime>
#include <iomanip>

VideoSourceImpl::VideoSourceImpl()
{
    pipeline   = nullptr;
    start_time = std::chrono::system_clock::now();
}

// Implement InitializeCameraDevice function
CameraError VideoSourceImpl::InitializeCameraDevice()
{
    // Implement the initialization logic for the camera hardware
    // Return CameraError::SUCCESS if initialization is successful, otherwise return an appropriate error code
    return CameraError::SUCCESS;
}

// Implement InitializeStreams function
CameraError VideoSourceImpl::InitializeStreams()
{
    // Implement the initialization logic for the camera streams
    // Return CameraError::SUCCESS if initialization is successful, otherwise return an appropriate error code
    return CameraError::SUCCESS;
}

// Callback function for processing frames
GstFlowReturn VideoSourceImpl::on_new_sample(GstAppSink * appsink, gpointer user_data)
{
    GstSample * sample = gst_app_sink_pull_sample(appsink);
    if (sample)
    {
        GstBuffer * buffer = gst_sample_get_buffer(sample);
        GstMapInfo map;
        if (gst_buffer_map(buffer, &map, GST_MAP_READ))
        {
            static int frame_count = 0;
            ++frame_count;
            std::time_t now_c = std::chrono::system_clock::to_time_t(start_time);
            std::tm * now_tm  = std::localtime(&now_c);
            std::cout << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S") << " Frame: " << frame_count
                      << " Received H.264 frame, size: " << map.size << " bytes" << std::endl;
            // Send video buffers to transport
            // video_frame_callback(map.data, map.size);
            gst_buffer_unmap(buffer, &map);
        }
        else
        {
            std::cerr << "Failed to map buffer" << std::endl;
        }
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }
    else
    {
        std::cerr << "Failed to pull sample" << std::endl;
        return GST_FLOW_ERROR;
    }
}

// Implement VideoStreamAllocate function
CameraError VideoSourceImpl::VideoStreamAllocate(const VideoStreamStruct & videoStreamParams, uint16_t & outStreamID)
{

    video_stream_params = videoStreamParams;

    // Create pipeline
    pipeline = (GstElement *) gst_pipeline_new("video-pipeline");
    if (!pipeline)
    {
        std::cerr << "Failed to create pipeline!" << std::endl;
        return -1;
    }

    // Create elements
    GstElement * videotestsrc = gst_element_factory_make("videotestsrc", "source");
    GstElement * capsfilter   = gst_element_factory_make("capsfilter", "filter");
    GstElement * x264enc      = gst_element_factory_make("x264enc", "encoder");
    GstElement * h264parse    = gst_element_factory_make("h264parse", "h264parse");
    GstElement * appsink      = gst_element_factory_make("appsink", "sink");

    if (!videotestsrc || !capsfilter || !x264enc || !h264parse || !appsink)
    {
        std::cerr << "Failed to create GStreamer elements!" << std::endl;
        if (pipeline)
            gst_object_unref(pipeline);
        return CameraError::ERROR_VIDEO_STREAM_ALLOC_FAILED;
    }

    // Add elements to pipeline
    gst_bin_add_many(GST_BIN(pipeline), videotestsrc, capsfilter, x264enc, h264parse, appsink, NULL);

    // Configure videotestsrc
    g_object_set(videotestsrc, "pattern", 0, NULL); // Use default pattern

    // Set caps for capsfilter
    GstCaps * caps = gst_caps_new_simple("video/x-raw", "width", G_TYPE_INT, video_stream_params.minResolution.width, "height",
                                         G_TYPE_INT, video_stream_params.minResolution.height, "framerate", GST_TYPE_FRACTION,
                                         video_stream_params.minFrameRate, 1, NULL);
    g_object_set(G_OBJECT(capsfilter), "caps", caps);
    gst_caps_unref(caps);

    // Configure x264enc
    g_object_set(x264enc, "speed-preset", 1, // ultrafast
                 "tune", 4,                  // zerolatency
                 "byte-stream", true, NULL);

    // Configure appsink
    g_object_set(appsink, "emit-signals", TRUE, "drop", TRUE, "max-buffers", 1, "sync", FALSE, NULL);

    // Set appsink callbacks
    GstAppSinkCallbacks callbacks = { nullptr, nullptr, VideoSourceImpl::on_new_sample };
    gst_app_sink_set_callbacks(GST_APP_SINK(appsink), &callbacks, nullptr, nullptr);

    // Link elements together with error checking
    if (!gst_element_link(videotestsrc, capsfilter))
    {
        std::cerr << "Failed to link videotestsrc -> capsfilter!" << std::endl;
        gst_object_unref(pipeline);
        return CameraError::ERROR_VIDEO_STREAM_ALLOC_FAILED;
    }
    if (!gst_element_link(capsfilter, x264enc))
    {
        std::cerr << "Failed to link capsfilter -> x264enc!" << std::endl;
        gst_object_unref(pipeline);
        return CameraError::ERROR_VIDEO_STREAM_ALLOC_FAILED;
    }

    if (!gst_element_link(x264enc, h264parse))
    {
        std::cerr << "Failed to link x264enc -> h264parse!" << std::endl;
        gst_object_unref(pipeline);
        return CameraError::ERROR_VIDEO_STREAM_ALLOC_FAILED;
    }

    return CameraError::SUCCESS;
}

// Implement AudioStreamAllocate function
CameraError VideoSourceImpl::AudioStreamAllocate(const AudioStreamStruct & audioStreamParams, uint16_t & outStreamID)
{
    // Implement the logic to allocate an audio stream
    // Return CameraError::SUCCESS if allocation is successful, otherwise return an appropriate error code
    return CameraError::SUCCESS;
}

// Implement SnapshotStreamAllocate function
CameraError VideoSourceImpl::SnapshotStreamAllocate(const SnapshotStreamStruct & snapshotStreamParams, uint16_t & outStreamID)
{
    // Implement the logic to allocate a snapshot stream
    // Return CameraError::SUCCESS if allocation is successful, otherwise return an appropriate error code
    return CameraError::SUCCESS;
}

// Implement VideoStreamDeallocate function
CameraError VideoSourceImpl::VideoStreamDeallocate(uint16_t streamID)
{
    if (pipeline)
    {
        gst_object_unref(pipeline);
    }
    return CameraError::SUCCESS;
}

// Implement AudioStreamDeallocate function
CameraError VideoSourceImpl::AudioStreamDeallocate(uint16_t streamID)
{
    // Implement the logic to deallocate an audio stream
    // Return CameraError::SUCCESS if deallocation is successful, otherwise return an appropriate error code
    return CameraError::SUCCESS;
}

// Implement SnapshotStreamDeallocate function
CameraError VideoSourceImpl::SnapshotStreamDeallocate(uint16_t streamID)
{
    // Implement the logic to deallocate a snapshot stream
    // Return CameraError::SUCCESS if deallocation is successful, otherwise return an appropriate error code
    return CameraError::SUCCESS;
}

// Implement CaptureSnapshot function
CameraError VideoSourceImpl::CaptureSnapshot(uint16_t streamID, const VideoResolutionStruct & resolution,
                                             ImageSnapshot & outImageSnapshot)
{
    // Implement the logic to capture a snapshot image
    // Populate outImageSnapshot with the captured image data and return CameraError::SUCCESS if successful
    return CameraError::SUCCESS;
}

// Implement StartVideoStream function
CameraError VideoSourceImpl::StartVideoStream(uint16_t streamID)
{
    // Start the pipeline
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        std::cerr << "Failed to set pipeline to PLAYING state!" << std::endl;
        gst_object_unref(pipeline);
        return CameraError::ERROR_VIDEO_STREAM_START_FAILED;
    }

    return CameraError::SUCCESS;
}

// Implement StopVideoStream function
CameraError VideoSourceImpl::StopVideoStream(uint16_t streamID)
{
    int ret = gst_element_set_state(pipeline, GST_STATE_NULL);
    if (ret != 0)
    {
        return CameraError::ERROR_VIDEO_STREAM_STOP_FAILED;
    }

    return CameraError::SUCCESS;
}

// Implement StartAudioStream function
CameraError VideoSourceImpl::StartAudioStream(uint16_t streamID)
{
    // Implement the logic to start an audio stream
    // Return CameraError::SUCCESS if starting is successful, otherwise return an appropriate error code
    return CameraError::SUCCESS;
}

// Implement StopAudioStream function
CameraError VideoSourceImpl::StopAudioStream(uint16_t streamID)
{
    // Implement the logic to stop an audio stream
    // Return CameraError::SUCCESS if stopping is successful, otherwise return an appropriate error code
    return CameraError::SUCCESS;
}

// Implement StartSnapshotStream function
CameraError VideoSourceImpl::StartSnapshotStream(uint16_t streamID)
{
    // Implement the logic to start a snapshot stream
    // Return CameraError::SUCCESS if starting is successful, otherwise return an appropriate error code
    return CameraError::SUCCESS;
}

// Implement StopSnapshotStream function
CameraError VideoSourceImpl::StopSnapshotStream(uint16_t streamID)
{
    // Implement the logic to stop a snapshot stream
    // Return CameraError::SUCCESS if stopping is successful, otherwise return an appropriate error code
    return CameraError::SUCCESS;
}

// Implement GetVideoSensorParams function
VideoSensorParamsStruct & VideoSourceImpl::GetVideoSensorParams()
{
    // Return a reference to the video sensor parameters
    static VideoSensorParamsStruct params;
    return params;
}

// Implement GetNightVisionCapable function
bool VideoSourceImpl::GetNightVisionCapable()
{
    // Return true if night vision is capable, otherwise return false
    return true;
}

// Implement GetMinViewport function
VideoResolutionStruct & VideoSourceImpl::GetMinViewport()
{
    // Return a reference to the minimum viewport resolution
    static VideoResolutionStruct resolution;
    return resolution;
}

// Implement GetMaxConcurrentVideoEncoders function
uint8_t VideoSourceImpl::GetMaxConcurrentVideoEncoders()
{
    // Return the maximum number of concurrent video encoders supported
    return 1;
}

// Implement GetMaxEncodedPixelRate function
uint32_t VideoSourceImpl::GetMaxEncodedPixelRate()
{
    return video_stream_params.maxResolution.width * video_stream_params.maxResolution.height * video_stream_params.maxFrameRate;
}

// Implement GetFrameRate function
uint16_t VideoSourceImpl::GetFrameRate()
{
    // Return the frame rate supported by the camera
    return video_stream_params.minFrameRate;
}

// Implement SetHDRMode function
void VideoSourceImpl::SetHDRMode(bool hdrMode)
{
    video_stream_params.hdrMode = hdrMode;
}
