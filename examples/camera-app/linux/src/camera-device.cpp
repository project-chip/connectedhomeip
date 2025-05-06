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
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;

using namespace Camera;

// Using Gstreamer video test source's ball animation pattern for the live streaming visual verification.
// Refer https://gstreamer.freedesktop.org/documentation/videotestsrc/index.html?gi-language=c#GstVideoTestSrcPattern

const int kBallAnimationPattern = 18;

CameraDevice::CameraDevice()
{
    InitializeCameraDevice();

    InitializeStreams();

    // Initialize Video Sources
    mNetworkVideoSource.Init(&mMediaController, VIDEO_STREAM_GST_DEST_PORT, StreamType::kVideo);

    // Initialize Audio Sources
    mNetworkAudioSource.Init(&mMediaController, AUDIO_STREAM_GST_DEST_PORT, StreamType::kAudio);

    // Initialize WebRTC connnection
    mWebRTCProviderManager.Init();

    // Set the CameraHALInterface in CameraAVStreamManager and CameraAVsettingsUserLevelManager.
    mCameraAVStreamManager.SetCameraDeviceHAL(this);
    mCameraAVSettingsUserLevelManager.SetCameraDeviceHAL(this);

    // Provider manager uses the Media controller to register WebRTC Transport with media controller for AV source data
    mWebRTCProviderManager.SetMediaController(&mMediaController);

    mCameraAVSettingsUserLevelManager.SetCameraDeviceHAL(this);
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

    // TODO: Replace hardcoded device file with device passed in from
    // camera-app.
    videoDeviceFd = open("/dev/video0", O_RDWR);
    if (videoDeviceFd == -1)
    {
        ChipLogError(Camera, "Error opening video device: %s", strerror(errno));
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

// Function to create the GStreamer pipeline
GstElement * CameraDevice::CreateSnapshotPipeline(const std::string & device, int width, int height, int quality, int framerate,
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
        ChipLogError(Camera, "Not all elements could be created.");
        gst_object_unref(pipeline);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    // Add elements to the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, jpeg_caps, videorate, videorate_caps, queue, filesink, NULL);

    // Link the elements
    if (gst_element_link_many(source, jpeg_caps, videorate, videorate_caps, queue, filesink, NULL) != TRUE)
    {
        ChipLogError(Camera, "Elements could not be linked.");
        gst_object_unref(pipeline);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    // Set the source device and caps
    g_object_set(source, "device", device.c_str(), "do-timestamp", TRUE, NULL);

    GstCaps * caps = gst_caps_new_simple("image/jpeg", "width", G_TYPE_INT, width, "height", G_TYPE_INT, height, "framerate",
                                         GST_TYPE_FRACTION, framerate, 1, "quality", G_TYPE_INT, quality, NULL);
    g_object_set(jpeg_caps, "caps", caps, NULL);
    gst_caps_unref(caps);

    // Set the output file location
    g_object_set(filesink, "location", filename.c_str(), NULL);

    return pipeline;
}

// Helper function to create a GStreamer pipeline
GstElement * CameraDevice::CreateVideoPipeline(const std::string & device, int width, int height, int framerate,
                                               CameraError & error)
{
    GstElement * pipeline     = nullptr;
    GstElement * source       = nullptr;
    GstElement * capsfilter   = nullptr;
    GstElement * videoconvert = nullptr;
    GstElement * x264enc      = nullptr;
    GstElement * rtph264pay   = nullptr;
    GstElement * udpsink      = nullptr;

    // Create the pipeline elements
    pipeline = gst_pipeline_new("video-pipeline");

    // Create elements
#ifdef AV_STREAM_GST_USE_TEST_SRC
    source = gst_element_factory_make("videotestsrc", "source");
#else
    source = gst_element_factory_make("v4l2src", "source");
#endif
    capsfilter   = gst_element_factory_make("capsfilter", "filter");
    videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    x264enc      = gst_element_factory_make("x264enc", "encoder");
    rtph264pay   = gst_element_factory_make("rtph264pay", "rtph264");
    udpsink      = gst_element_factory_make("udpsink", "udpsink");

    if (pipeline == nullptr || source == nullptr || capsfilter == nullptr || videoconvert == nullptr || x264enc == nullptr ||
        rtph264pay == nullptr || udpsink == nullptr)
    {
        ChipLogError(Camera, "Not all elements could be created.");
        if (pipeline)
            gst_object_unref(pipeline);
        if (source)
            gst_object_unref(source);
        if (capsfilter)
            gst_object_unref(capsfilter);
        if (videoconvert)
            gst_object_unref(videoconvert);
        if (x264enc)
            gst_object_unref(x264enc);
        if (rtph264pay)
            gst_object_unref(rtph264pay);
        if (udpsink)
            gst_object_unref(udpsink);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    // Add elements to the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, capsfilter, videoconvert, x264enc, rtph264pay, udpsink, NULL);

    // Link the elements
    if (!gst_element_link_many(source, capsfilter, videoconvert, x264enc, rtph264pay, udpsink, NULL))
    {
        ChipLogError(Camera, "Elements could not be linked.");
        if (pipeline)
            gst_object_unref(pipeline);
        if (source)
            gst_object_unref(source);
        if (capsfilter)
            gst_object_unref(capsfilter);
        if (videoconvert)
            gst_object_unref(videoconvert);
        if (x264enc)
            gst_object_unref(x264enc);
        if (rtph264pay)
            gst_object_unref(rtph264pay);
        if (udpsink)
            gst_object_unref(udpsink);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    // Create GstCaps for the video source
    GstCaps * caps = gst_caps_new_simple("video/x-raw", "width", G_TYPE_INT, width, "height", G_TYPE_INT, height, "format",
                                         G_TYPE_STRING, "NV12", // Adjust format as needed
                                         "framerate", GST_TYPE_FRACTION, framerate, 1, NULL);

    // Set video test src pattern
#ifdef AV_STREAM_GST_USE_TEST_SRC
    g_object_set(source, "pattern", kBallAnimationPattern, NULL);
#endif

    // Set the caps on the capsfilter element
    g_object_set(capsfilter, "caps", caps, NULL);

    // Set udpsink properties
    g_object_set(udpsink, "host", STREAM_GST_DEST_IP, "port", VIDEO_STREAM_GST_DEST_PORT, NULL);

    // Unref the caps to free memory
    gst_caps_unref(caps);

    return pipeline;
}

// Helper function to create a GStreamer pipeline
GstElement * CameraDevice::CreateAudioPipeline(const std::string & device, int channels, int sampleRate, CameraError & error)
{
    GstElement * pipeline     = nullptr;
    GstElement * source       = nullptr;
    GstElement * capsfilter   = nullptr;
    GstElement * audioconvert = nullptr;
    GstElement * opusenc      = nullptr;
    GstElement * rtpopuspay   = nullptr;
    GstElement * udpsink      = nullptr;

    // Create the pipeline elements
    pipeline = gst_pipeline_new("audio-pipeline");

    // Create elements
#ifdef AV_STREAM_GST_USE_TEST_SRC
    source = gst_element_factory_make("audiotestsrc", "source");
#else
    source = gst_element_factory_make("pulsesrc", "source");
#endif
    capsfilter   = gst_element_factory_make("capsfilter", "filter");
    audioconvert = gst_element_factory_make("audioconvert", "audio-convert");
    opusenc      = gst_element_factory_make("opusenc", "opus-encoder");
    rtpopuspay   = gst_element_factory_make("rtpopuspay", "rtpopuspay");
    udpsink      = gst_element_factory_make("udpsink", "udpsink");

    if (pipeline == nullptr || source == nullptr || capsfilter == nullptr || audioconvert == nullptr || opusenc == nullptr ||
        rtpopuspay == nullptr || udpsink == nullptr)
    {
        ChipLogError(Camera, "Not all elements could be created.");
        if (pipeline)
            gst_object_unref(pipeline);
        if (source)
            gst_object_unref(source);
        if (capsfilter)
            gst_object_unref(capsfilter);
        if (audioconvert)
            gst_object_unref(audioconvert);
        if (opusenc)
            gst_object_unref(opusenc);
        if (rtpopuspay)
            gst_object_unref(rtpopuspay);
        if (udpsink)
            gst_object_unref(udpsink);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    // Create GstCaps for the audio source
    GstCaps * caps = gst_caps_new_simple("audio/x-raw", "channels", G_TYPE_INT, channels, "rate", G_TYPE_INT, sampleRate, NULL);

    // Set the caps on the capsfilter element
    g_object_set(capsfilter, "caps", caps, NULL);

    // Set udpsink properties
    g_object_set(udpsink, "host", STREAM_GST_DEST_IP, "port", AUDIO_STREAM_GST_DEST_PORT, NULL);

    // Unref the caps to free memory
    gst_caps_unref(caps);

    // Add elements to the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, capsfilter, audioconvert, opusenc, rtpopuspay, udpsink, NULL);

    // Link elements
    if (!gst_element_link_many(source, capsfilter, audioconvert, opusenc, rtpopuspay, udpsink, NULL))
    {
        ChipLogError(Camera, "Elements could not be linked.");
        if (pipeline)
            gst_object_unref(pipeline);
        if (source)
            gst_object_unref(source);
        if (capsfilter)
            gst_object_unref(capsfilter);
        if (audioconvert)
            gst_object_unref(audioconvert);
        if (opusenc)
            gst_object_unref(opusenc);
        if (rtpopuspay)
            gst_object_unref(rtpopuspay);
        if (udpsink)
            gst_object_unref(udpsink);
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
        ChipLogError(Camera, "Error setting V4L2 control: %s", strerror(errno));

        return CameraError::ERROR_CONFIG_FAILED;
    }

    return CameraError::SUCCESS;
}

CameraError CameraDevice::CaptureSnapshot(const chip::app::DataModel::Nullable<uint16_t> streamID,
                                          const VideoResolutionStruct & resolution, ImageSnapshot & outImageSnapshot)
{
    uint16_t streamId = streamID.IsNull() ? 1 : streamID.Value();
    auto it           = std::find_if(snapshotStreams.begin(), snapshotStreams.end(),
                                     [streamId](const SnapshotStream & s) { return s.snapshotStreamParams.snapshotStreamID == streamId; });
    if (it == snapshotStreams.end())
    {
        ChipLogError(Camera, "Snapshot streamID : %u not found", streamId);
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    // Read from image file stored from snapshot stream.
    std::ifstream file(SNAPSHOT_FILE_PATH, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        ChipLogError(Camera, "Error opening snapshot image file: ");
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Ensure space for image snapshot data in outImageSnapshot
    outImageSnapshot.data.resize(static_cast<size_t>(size));

    if (!file.read(reinterpret_cast<char *>(outImageSnapshot.data.data()), size))
    {
        ChipLogError(Camera, "Error reading image file: ");
        file.close();
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    file.close();

    outImageSnapshot.imageRes.width  = it->snapshotStreamParams.minResolution.width;
    outImageSnapshot.imageRes.height = it->snapshotStreamParams.minResolution.height;
    outImageSnapshot.imageCodec      = it->snapshotStreamParams.imageCodec;

    return CameraError::SUCCESS;
}

CameraError CameraDevice::StartVideoStream(uint16_t streamID)
{
    auto it = std::find_if(videoStreams.begin(), videoStreams.end(),
                           [streamID](const VideoStream & s) { return s.videoStreamParams.videoStreamID == streamID; });

    if (it == videoStreams.end())
    {
        return CameraError::ERROR_VIDEO_STREAM_START_FAILED;
    }

    // Create Gstreamer video pipeline
    CameraError error = CameraError::SUCCESS;
    GstElement * videoPipeline =
        CreateVideoPipeline("/dev/video0", it->videoStreamParams.minResolution.width, it->videoStreamParams.minResolution.height,
                            it->videoStreamParams.minFrameRate, error);
    if (videoPipeline == nullptr)
    {
        ChipLogError(Camera, "Failed to create video pipeline.");
        it->videoContext = nullptr;
        return CameraError::ERROR_VIDEO_STREAM_START_FAILED;
    }

    // Start the pipeline
    GstStateChangeReturn result = gst_element_set_state(videoPipeline, GST_STATE_PLAYING);
    if (result == GST_STATE_CHANGE_FAILURE)
    {
        ChipLogError(Camera, "Failed to start video pipeline.");
        gst_object_unref(videoPipeline);
        it->videoContext = nullptr;
        return CameraError::ERROR_VIDEO_STREAM_START_FAILED;
    }

    // Start the pipeline
    result = gst_element_set_state(videoPipeline, GST_STATE_PLAYING);
    if (result == GST_STATE_CHANGE_FAILURE)
    {
        ChipLogError(Camera, "Failed to start video pipeline.");
        gst_object_unref(videoPipeline);
        it->videoContext = nullptr;
        return CameraError::ERROR_VIDEO_STREAM_START_FAILED;
    }

    // Wait for the pipeline to reach the PLAYING state
    GstState state;
    gst_element_get_state(videoPipeline, &state, nullptr, GST_CLOCK_TIME_NONE);
    if (state != GST_STATE_PLAYING)
    {
        ChipLogError(Camera, "Video pipeline did not reach PLAYING state.");
        gst_element_set_state(videoPipeline, GST_STATE_NULL);
        gst_object_unref(videoPipeline);
        it->videoContext = nullptr;
        return CameraError::ERROR_VIDEO_STREAM_START_FAILED;
    }

    // Start the network stream source after the Gstreamer pipeline is setup
    mNetworkVideoSource.Start(streamID);

    // Store in stream context
    it->videoContext = videoPipeline;

    return CameraError::SUCCESS;
}

// Stop video stream
CameraError CameraDevice::StopVideoStream(uint16_t streamID)
{
    auto it = std::find_if(videoStreams.begin(), videoStreams.end(),
                           [streamID](const VideoStream & s) { return s.videoStreamParams.videoStreamID == streamID; });

    if (it == videoStreams.end())
    {
        return CameraError::ERROR_VIDEO_STREAM_STOP_FAILED;
    }

    GstElement * videoPipeline = reinterpret_cast<GstElement *>(it->videoContext);
    if (videoPipeline != nullptr)
    {
        GstStateChangeReturn result = gst_element_set_state(videoPipeline, GST_STATE_NULL);
        if (result == GST_STATE_CHANGE_FAILURE)
        {
            return CameraError::ERROR_VIDEO_STREAM_STOP_FAILED;
        }

        gst_element_set_state(videoPipeline, GST_STATE_NULL);
        gst_object_unref(videoPipeline);
        it->videoContext = nullptr;
    }

    return CameraError::SUCCESS;
}

// Start audio stream
CameraError CameraDevice::StartAudioStream(uint16_t streamID)
{
    auto it = std::find_if(audioStreams.begin(), audioStreams.end(),
                           [streamID](const AudioStream & s) { return s.audioStreamParams.audioStreamID == streamID; });

    if (it == audioStreams.end())
    {
        ChipLogError(Camera, "Audio streamID : %u not found", streamID);
        return CameraError::ERROR_AUDIO_STREAM_START_FAILED;
    }

    int channels   = it->audioStreamParams.channelCount;
    int sampleRate = it->audioStreamParams.sampleRate;

    // Create Gstreamer video pipeline
    CameraError error          = CameraError::SUCCESS;
    GstElement * audioPipeline = CreateAudioPipeline("/dev/audio0", channels, sampleRate, error);
    if (audioPipeline == nullptr)
    {
        ChipLogError(Camera, "Failed to create audio pipeline.");
        it->audioContext = nullptr;
        return CameraError::ERROR_AUDIO_STREAM_START_FAILED;
    }

    // Start the pipeline
    GstStateChangeReturn result = gst_element_set_state(audioPipeline, GST_STATE_PLAYING);
    if (result == GST_STATE_CHANGE_FAILURE)
    {
        ChipLogError(Camera, "Failed to start audio pipeline.");
        gst_object_unref(audioPipeline);
        it->audioContext = nullptr;
        return CameraError::ERROR_AUDIO_STREAM_START_FAILED;
    }

    // Wait for the pipeline to reach the PLAYING state
    GstState state;
    gst_element_get_state(audioPipeline, &state, nullptr, GST_CLOCK_TIME_NONE);
    if (state != GST_STATE_PLAYING)
    {
        ChipLogError(Camera, "Audio pipeline did not reach PLAYING state.");
        gst_element_set_state(audioPipeline, GST_STATE_NULL);
        gst_object_unref(audioPipeline);
        it->audioContext = nullptr;
        return CameraError::ERROR_AUDIO_STREAM_START_FAILED;
    }

    // Start the network stream source after the Gstreamer pipeline is setup
    mNetworkAudioSource.Start(streamID);

    // Store in stream context
    it->audioContext = audioPipeline;

    return CameraError::SUCCESS;
}

// Stop audio stream
CameraError CameraDevice::StopAudioStream(uint16_t streamID)
{
    auto it = std::find_if(audioStreams.begin(), audioStreams.end(),
                           [streamID](const AudioStream & s) { return s.audioStreamParams.audioStreamID == streamID; });

    if (it == audioStreams.end())
    {
        return CameraError::ERROR_AUDIO_STREAM_STOP_FAILED;
    }

    GstElement * audioPipeline = reinterpret_cast<GstElement *>(it->audioContext);
    if (audioPipeline != nullptr)
    {
        GstStateChangeReturn result = gst_element_set_state(audioPipeline, GST_STATE_NULL);
        if (result == GST_STATE_CHANGE_FAILURE)
        {
            return CameraError::ERROR_SNAPSHOT_STREAM_STOP_FAILED;
        }
        gst_object_unref(audioPipeline);
        it->audioContext = nullptr;
    }

    return CameraError::SUCCESS;
}

// Start snapshot stream
CameraError CameraDevice::StartSnapshotStream(uint16_t streamID)
{
    auto it = std::find_if(snapshotStreams.begin(), snapshotStreams.end(),
                           [streamID](const SnapshotStream & s) { return s.snapshotStreamParams.snapshotStreamID == streamID; });
    if (it == snapshotStreams.end())
    {
        ChipLogError(Camera, "Snapshot streamID : %u not found", streamID);
        return CameraError::ERROR_SNAPSHOT_STREAM_START_FAILED;
    }

    // Create the GStreamer pipeline
    CameraError error             = CameraError::SUCCESS;
    GstElement * snapshotPipeline = CreateSnapshotPipeline(
        "/dev/video0", it->snapshotStreamParams.minResolution.width, it->snapshotStreamParams.minResolution.height,
        it->snapshotStreamParams.quality, it->snapshotStreamParams.frameRate, "capture_snapshot.jpg", error);
    if (snapshotPipeline == nullptr)
    {
        ChipLogError(Camera, "Failed to create snapshot pipeline.");
        it->snapshotContext = nullptr;
        return error;
    }

    // Start the pipeline
    GstStateChangeReturn result = gst_element_set_state(snapshotPipeline, GST_STATE_PLAYING);
    if (result == GST_STATE_CHANGE_FAILURE)
    {
        ChipLogError(Camera, "Failed to start snapshot pipeline.");
        gst_object_unref(snapshotPipeline);
        it->snapshotContext = nullptr;
        return CameraError::ERROR_SNAPSHOT_STREAM_START_FAILED;
    }

    // Wait for the pipeline to reach the PLAYING state
    GstState state;
    gst_element_get_state(snapshotPipeline, &state, nullptr, GST_CLOCK_TIME_NONE);
    if (state != GST_STATE_PLAYING)
    {
        ChipLogError(Camera, "Snapshot pipeline did not reach PLAYING state.");
        gst_element_set_state(snapshotPipeline, GST_STATE_NULL);
        gst_object_unref(snapshotPipeline);
        it->snapshotContext = nullptr;
        return CameraError::ERROR_SNAPSHOT_STREAM_START_FAILED;
    }

    // Store in stream context
    it->snapshotContext = snapshotPipeline;

    return CameraError::SUCCESS;
}

// Stop snapshot stream
CameraError CameraDevice::StopSnapshotStream(uint16_t streamID)
{
    auto it = std::find_if(snapshotStreams.begin(), snapshotStreams.end(),
                           [streamID](const SnapshotStream & s) { return s.snapshotStreamParams.snapshotStreamID == streamID; });
    if (it == snapshotStreams.end())
    {
        return CameraError::ERROR_SNAPSHOT_STREAM_STOP_FAILED;
    }

    GstElement * snapshotPipeline = reinterpret_cast<GstElement *>(it->snapshotContext);
    if (snapshotPipeline != nullptr)
    {
        // Stop the pipeline
        GstStateChangeReturn result = gst_element_set_state(snapshotPipeline, GST_STATE_NULL);
        if (result == GST_STATE_CHANGE_FAILURE)
        {
            return CameraError::ERROR_SNAPSHOT_STREAM_STOP_FAILED;
        }

        // Unreference the pipeline
        gst_object_unref(snapshotPipeline);
        it->snapshotContext = nullptr;
    }

    // Remove the snapshot file
    std::string fileName = SNAPSHOT_FILE_PATH;
    if (unlink(fileName.c_str()) == -1)
    {
        ChipLogError(Camera, "Failed to remove snapshot file after stopping stream (err = %s).", strerror(errno));
        return CameraError::ERROR_SNAPSHOT_STREAM_STOP_FAILED;
    }

    return CameraError::SUCCESS;
}

uint8_t CameraDevice::GetMaxConcurrentEncoders()
{
    return kMaxConcurrentEncoders;
}

uint32_t CameraDevice::GetMaxEncodedPixelRate()
{
    return kMaxEncodedPixelRate;
}

VideoSensorParamsStruct & CameraDevice::GetVideoSensorParams()
{
    static VideoSensorParamsStruct videoSensorParams = { kVideoSensorWidthPixels, kVideoSensorHeightPixels, kMaxVideoFrameRate,
                                                         chip::Optional<uint16_t>(30) }; // Typical numbers for Pi camera.
    return videoSensorParams;
}

bool CameraDevice::GetCameraSupportsNightVision()
{
    return true;
}

bool CameraDevice::GetNightVisionUsesInfrared()
{
    return false;
}

VideoResolutionStruct & CameraDevice::GetMinViewport()
{
    static VideoResolutionStruct minViewport = { kMinResolutionWidth, kMinResolutionHeight };
    return minViewport;
}

std::vector<RateDistortionTradeOffStruct> & CameraDevice::GetRateDistortionTradeOffPoints()
{
    static std::vector<RateDistortionTradeOffStruct> rateDistTradeOffs = {
        { VideoCodecEnum::kH264, { kMinResolutionWidth, kMinResolutionHeight }, 10000 /* bitrate */ }
    };
    return rateDistTradeOffs;
}

uint32_t CameraDevice::GetMaxContentBufferSize()
{
    return kMaxContentBufferSizeBytes;
}

AudioCapabilitiesStruct & CameraDevice::GetMicrophoneCapabilities()
{
    static std::array<AudioCodecEnum, 2> audioCodecs = { AudioCodecEnum::kOpus, AudioCodecEnum::kAacLc };
    static std::array<uint32_t, 2> sampleRates       = { 48000, 32000 }; // Sample rates in Hz
    static std::array<uint8_t, 2> bitDepths          = { 24, 32 };
    static AudioCapabilitiesStruct audioCapabilities = { kMicrophoneMaxChannelCount, chip::Span<AudioCodecEnum>(audioCodecs),
                                                         chip::Span<uint32_t>(sampleRates), chip::Span<uint8_t>(bitDepths) };
    return audioCapabilities;
}

AudioCapabilitiesStruct & CameraDevice::GetSpeakerCapabilities()
{
    static AudioCapabilitiesStruct speakerCapabilities = {};
    return speakerCapabilities;
}

std::vector<SnapshotCapabilitiesStruct> & CameraDevice::GetSnapshotCapabilities()
{
    static std::vector<SnapshotCapabilitiesStruct> snapshotCapabilities = { { { kMinResolutionWidth, kMinResolutionHeight },
                                                                              kSnapshotStreamFrameRate,
                                                                              ImageCodecEnum::kJpeg,
                                                                              false,
                                                                              chip::MakeOptional(static_cast<bool>(false)) } };
    return snapshotCapabilities;
}

uint32_t CameraDevice::GetMaxNetworkBandwidth()
{
    return kMaxNetworkBandwidthMbps;
}

uint16_t CameraDevice::GetCurrentFrameRate()
{
    return mCurrentVideoFrameRate;
}

CameraError CameraDevice::SetHDRMode(bool hdrMode)
{
    mHDREnabled = hdrMode;

    return CameraError::SUCCESS;
}

std::vector<StreamUsageEnum> & CameraDevice::GetSupportedStreamUsages()
{
    static std::vector<StreamUsageEnum> supportedStreamUsage = { StreamUsageEnum::kLiveView, StreamUsageEnum::kRecording };
    return supportedStreamUsage;
}

CameraError CameraDevice::SetViewport(const ViewportStruct & viewPort)
{
    mViewport = viewPort;

    return CameraError::SUCCESS;
}

CameraError CameraDevice::SetViewport(VideoStream & stream, const ViewportStruct & viewport)
{
    ChipLogDetail(Camera, "Setting per stream viewport for stream %d.", stream.videoStreamParams.videoStreamID);
    ChipLogDetail(Camera, "New viewport. x1=%d, x2=%d, y1=%d, y2=%d.", viewport.x1, viewport.x2, viewport.y1, viewport.y2);
    stream.viewport = viewport;
    return CameraError::SUCCESS;
}

// Mute/Unmute microphone.
CameraError CameraDevice::SetMicrophoneMuted(bool muteMicrophone)
{
    mMicrophoneMuted = muteMicrophone;

    return CameraError::SUCCESS;
}

// Set microphone volume level.
CameraError CameraDevice::SetMicrophoneVolume(uint8_t microphoneVol)
{
    mMicrophoneVol = microphoneVol;

    return CameraError::SUCCESS;
}

int16_t CameraDevice::GetPanMin()
{
    return kMinPanValue;
}

int16_t CameraDevice::GetPanMax()
{
    return kMaxPanValue;
}

int16_t CameraDevice::GetTiltMin()
{
    return kMinTiltValue;
}

int16_t CameraDevice::GetTiltMax()
{
    return kMaxTiltValue;
}

uint8_t CameraDevice::GetZoomMax()
{
    return kMaxZoomValue;
}

// Set the Pan level
CameraError CameraDevice::SetPan(int16_t aPan)
{
    mPan = aPan;
    return CameraError::SUCCESS;
}

// Set the Tilt level
CameraError CameraDevice::SetTilt(int16_t aTilt)
{
    mTilt = aTilt;
    return CameraError::SUCCESS;
}

// Set the Zoom level
CameraError CameraDevice::SetZoom(uint8_t aZoom)
{
    mZoom = aZoom;
    return CameraError::SUCCESS;
}

void CameraDevice::InitializeVideoStreams()
{
    // Create single video stream with typical supported parameters
    VideoStream videoStream = { { 1 /* Id */,
                                  StreamUsageEnum::kLiveView /* StreamUsage */,
                                  VideoCodecEnum::kH264,
                                  kMinVideoFrameRate /* MinFrameRate */,
                                  kMaxVideoFrameRate /* MaxFrameRate */,
                                  { kMinResolutionWidth, kMinResolutionHeight } /* MinResolution */,
                                  { kMaxResolutionWidth, kMaxResolutionHeight } /* MaxResolution */,
                                  kMinBitRateBps /* MinBitRate */,
                                  kMaxBitRateBps /* MaxBitRate */,
                                  kMinFragLenMsec /* MinFragmentLen */,
                                  kMaxFragLenMsec /* MaxFragmentLen */,
                                  chip::MakeOptional(static_cast<bool>(false)) /* WMark */,
                                  chip::MakeOptional(static_cast<bool>(false)) /* OSD */,
                                  0 /* RefCount */ },
                                false,
                                { mViewport.x1, mViewport.y1, mViewport.x2, mViewport.y2 },
                                nullptr };

    videoStreams.push_back(videoStream);
}

void CameraDevice::InitializeAudioStreams()
{
    // Create single audio stream with typical supported parameters
    AudioStream audioStream = { { 1 /* Id */, StreamUsageEnum::kLiveView /* StreamUsage */, AudioCodecEnum::kOpus,
                                  2 /* ChannelCount */, 48000 /* SampleRate */, 20000 /* BitRate*/, 24 /* BitDepth */,
                                  0 /* RefCount */ },
                                false,
                                nullptr };

    audioStreams.push_back(audioStream);
}

void CameraDevice::InitializeSnapshotStreams()
{
    // Create single snapshot stream with typical supported parameters
    SnapshotStream snapshotStream = { { 1 /* Id */,
                                        ImageCodecEnum::kJpeg,
                                        kSnapshotStreamFrameRate /* FrameRate */,
                                        { kMinResolutionWidth, kMinResolutionHeight } /* MinResolution*/,
                                        { kMaxResolutionWidth, kMaxResolutionHeight } /* MaxResolution */,
                                        90 /* Quality */,
                                        0 /* RefCount */ },
                                      false,
                                      nullptr };

    snapshotStreams.push_back(snapshotStream);
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

CameraAvSettingsUserLevelManagement::Delegate & CameraDevice::GetCameraAVSettingsUserLevelMgmtDelegate()
{
    return mCameraAVSettingsUserLevelManager;
}

MediaController & CameraDevice::GetMediaController()
{
    return mMediaController;
}
