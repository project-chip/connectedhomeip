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
#include <Options.h>
#include <chrono>
#include <fcntl.h> // For file descriptor operations
#include <filesystem>
#include <fstream>
#include <gst/app/gstappsink.h>
#include <gst/gst.h>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>
#include <limits.h>          // For PATH_MAX
#include <linux/videodev2.h> // For V4L2 definitions
#include <sys/ioctl.h>

// File used to store snapshot from stream and return for CaptureSnapshot
// command.
#define SNAPSHOT_FILE_PATH "./capture_snapshot.jpg"
// Timeout for video pipeline to go to playing state.
#define VIDEO_PIPELINE_PLAY_TIMEOUT 5
// Framesize for audio pipeline
#define AUDIO_FRAMESIZE 20

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Chime;
using namespace chip::app::Clusters::CameraAvStreamManagement;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;
using namespace chip::app::Clusters::WebRTCTransportProvider;
using namespace chip::app::Clusters::ZoneManagement;

using namespace Camera;

namespace {

// Context structure to pass both CameraDevice and videoStreamID to the callback
struct VideoAppSinkContext
{
    CameraDevice * device;
    uint16_t videoStreamID;
};

// Context structure to pass both CameraDevice and audioStreamID to the callback
struct AudioAppSinkContext
{
    CameraDevice * device;
    uint16_t audioStreamID;
};

// Using Gstreamer video test source's ball animation pattern for the live streaming visual verification.
// Refer https://gstreamer.freedesktop.org/documentation/videotestsrc/index.html?gi-language=c#GstVideoTestSrcPattern

// Callback function for GStreamer app sink
GstFlowReturn OnNewVideoSampleFromAppSink(GstAppSink * appsink, gpointer user_data)
{
    VideoAppSinkContext * context = static_cast<VideoAppSinkContext *>(user_data);
    CameraDevice * self           = context->device;
    uint16_t videoStreamID        = context->videoStreamID;

    GstSample * sample = gst_app_sink_pull_sample(appsink);
    if (sample == nullptr)
    {
        return GST_FLOW_ERROR;
    }

    GstBuffer * buffer = gst_sample_get_buffer(sample);
    if (buffer == nullptr)
    {
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_READ))
    {
        // Forward raw H.264 encoded frames to media controller
        // The PreRollBuffer will distribute to ALL transports registered for this videoStreamID
        // Each transport will handle its own SFrame encryption (if configured) during RTP packetization
        self->GetMediaController().DistributeVideo(reinterpret_cast<const uint8_t *>(map.data), map.size, videoStreamID);
        gst_buffer_unmap(buffer, &map);
    }

    gst_sample_unref(sample);
    return GST_FLOW_OK;
}

// Cleanup function for the context
void DestroyVideoAppSinkContext(gpointer user_data)
{
    VideoAppSinkContext * context = static_cast<VideoAppSinkContext *>(user_data);
    delete context;
}
static GstFlowReturn OnNewAudioSampleFromAppSink(GstAppSink * appsink, gpointer user_data)
{
    auto * context     = static_cast<AudioAppSinkContext *>(user_data);
    auto * self        = context->device;
    auto audioStreamID = context->audioStreamID;

    GstSample * sample = gst_app_sink_pull_sample(appsink);
    if (!sample)
        return GST_FLOW_ERROR;

    GstBuffer * buffer = gst_sample_get_buffer(sample);
    if (!buffer)
    {
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    // opusenc sends codec headers at start; ignore them
    if (GST_BUFFER_FLAG_IS_SET(buffer, GST_BUFFER_FLAG_HEADER))
    {
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }

    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_READ))
    {
        // Forward raw Opus encoded frames to media controller
        // The PreRollBuffer will distribute to ALL transports registered for this audioStreamID
        // Each transport will handle its own SFrame encryption (if configured) during RTP packetization
        self->GetMediaController().DistributeAudio(reinterpret_cast<const uint8_t *>(map.data), map.size, audioStreamID);
        gst_buffer_unmap(buffer, &map);
    }

    gst_sample_unref(sample);
    return GST_FLOW_OK;
}

static void DestroyAudioAppSinkContext(gpointer user_data)
{
    delete static_cast<AudioAppSinkContext *>(user_data);
}

} // namespace

namespace GstreamerPipepline {

enum class CameraType
{
    kCsi,
    kUsb,
    kFailure,
};

CameraType detectCameraType(const std::string & fullDevicePath)
{
    if (!std::filesystem::exists(fullDevicePath))
    {
        return CameraType::kFailure;
    }

    std::string videoDeviceName = std::filesystem::path(fullDevicePath).filename(); // ex: video0
    std::string sysPath         = "/sys/class/video4linux/" + videoDeviceName + "/device/driver/module";

    char resolvedPath[PATH_MAX];
    ssize_t len = readlink(sysPath.c_str(), resolvedPath, sizeof(resolvedPath) - 1);

    // Define driver name constants to avoid magic strings
    constexpr const char * kCsiDriver1 = "bm2835";
    constexpr const char * kCsiDriver2 = "unicam";
    constexpr const char * kUsbDriver  = "uvc";

    VerifyOrReturnError(len != -1, CameraType::kFailure);

    const std::string driverPath(resolvedPath, static_cast<size_t>(len));

    if (driverPath.find(kCsiDriver1) != std::string::npos || driverPath.find(kCsiDriver2) != std::string::npos)
    {
        return CameraType::kCsi;
    }
    if (driverPath.find(kUsbDriver) != std::string::npos)
    {
        return CameraType::kUsb;
    }

    return CameraType::kFailure;
}

// Function to unreference GStreamer elements and the pipeline
template <typename... Args>
void unrefGstElements(GstElement * pipeline, Args... elements)
{
    if (pipeline)
    {
        gst_object_unref(pipeline);
    }

    // Unreference each element in the variadic template argument pack
    ((elements ? gst_object_unref(elements) : void()), ...);
}

bool isGstElementsNull(const std::vector<std::pair<GstElement *, const char *>> & elements)
{
    bool isNull = false;

    // Check if any of the elements in the vector is nullptr
    for (const auto & element : elements)
    {
        if (!element.first)
        {
            ChipLogError(Camera, "Element '%s' could not be created.", element.second);
            isNull = true;
        }
    }

    return isNull;
}

namespace Snapshot {
struct SnapshotPipelineConfig
{
    std::string device;
    int width;
    int height;
    int quality;
    int framerate;
    std::string filename;
};

GstElement * CreateSnapshotPipelineV4l2(const SnapshotPipelineConfig & config, CameraError & error)
{
    // Create the GStreamer elements for the snapshot pipeline
    GstElement * pipeline = gst_pipeline_new("snapshot-pipeline");
    // TODO: Have the video source passed in.
    GstElement * source         = gst_element_factory_make("v4l2src", "source");
    GstElement * jpeg_caps      = gst_element_factory_make("capsfilter", "jpeg_caps");
    GstElement * videorate      = gst_element_factory_make("videorate", "videorate");
    GstElement * videorate_caps = gst_element_factory_make("capsfilter", "timelapse_framerate");
    GstElement * queue          = gst_element_factory_make("queue", "queue");
    GstElement * filesink       = gst_element_factory_make("multifilesink", "sink");

    // Check for any nullptr among the created elements
    const std::vector<std::pair<GstElement *, const char *>> elements = {
        { pipeline, "pipeline" },             //
        { source, "source" },                 //
        { jpeg_caps, "jpeg_caps" },           //
        { videorate, "videorate" },           //
        { videorate_caps, "videorate_caps" }, //
        { queue, "queue" },                   //
        { filesink, "filesink" }              //
    };
    bool isElementFactoryMakeFailed = GstreamerPipepline::isGstElementsNull(elements);

    // If any element creation failed, log the error and unreference the elements
    if (isElementFactoryMakeFailed)
    {
        // Unreference the elements that were created
        GstreamerPipepline::unrefGstElements(pipeline, source, jpeg_caps, videorate, videorate_caps, queue, filesink);

        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    // Set the source device and caps
    g_object_set(source, "device", config.device.c_str(), "do-timestamp", TRUE, nullptr);

    GstCaps * caps = gst_caps_new_simple(                    //
        "image/jpeg",                                        //
        "width", G_TYPE_INT, config.width,                   //
        "height", G_TYPE_INT, config.height,                 //
        "framerate", GST_TYPE_FRACTION, config.framerate, 1, //
        "quality", G_TYPE_INT, config.quality,               //
        nullptr                                              //
    );
    g_object_set(jpeg_caps, "caps", caps, nullptr);
    gst_caps_unref(caps);

    // Set the output file location
    g_object_set(filesink, "location", config.filename.c_str(), nullptr);

    // Add elements to the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, jpeg_caps, videorate, videorate_caps, queue, filesink, nullptr);

    // Link the elements
    if (gst_element_link_many(source, jpeg_caps, videorate, videorate_caps, queue, filesink, nullptr) != TRUE)
    {
        ChipLogError(Camera, "Elements could not be linked.");

        // The pipeline will unref all added elements automatically when you unref the pipeline.
        gst_object_unref(pipeline);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    return pipeline;
}

GstElement * CreateSnapshotPipelineLibcamerasrc(const SnapshotPipelineConfig & config, CameraError & error)
{
    // Create the GStreamer elements for the snapshot pipeline
    GstElement * pipeline   = gst_pipeline_new("snapshot-pipeline");
    GstElement * source     = gst_element_factory_make("libcamerasrc", "source");
    GstElement * capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
    GstElement * jpegenc    = gst_element_factory_make("jpegenc", "jpegenc");
    GstElement * queue      = gst_element_factory_make("queue", "queue");
    GstElement * filesink   = gst_element_factory_make("multifilesink", "sink");

    // Check for any nullptr among the created elements
    const std::vector<std::pair<GstElement *, const char *>> elements = {
        { pipeline, "pipeline" },     //
        { source, "source" },         //
        { capsfilter, "capsfilter" }, //
        { jpegenc, "jpegenc" },       //
        { queue, "queue" },           //
        { filesink, "filesink" }      //
    };
    const bool isElementFactoryMakeFailed = GstreamerPipepline::isGstElementsNull(elements);

    // If any element creation failed, log the error and unreference the elements
    if (isElementFactoryMakeFailed)
    {
        // Unreference the elements that were created
        GstreamerPipepline::unrefGstElements(pipeline, source, capsfilter, jpegenc, filesink);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    // Set resolution and framerate caps
    GstCaps * caps = gst_caps_new_simple(                    //
        "video/x-raw",                                       //
        "width", G_TYPE_INT, config.width,                   //
        "height", G_TYPE_INT, config.height,                 //
        "framerate", GST_TYPE_FRACTION, config.framerate, 1, //
        nullptr                                              //
    );
    g_object_set(capsfilter, "caps", caps, nullptr);
    gst_caps_unref(caps);

    // Set JPEG quality
    g_object_set(jpegenc, "quality", config.quality, nullptr);

    // Set multifilesink to write only one file
    g_object_set(filesink, "location", config.filename.c_str(), nullptr);

    // Add and link elements
    gst_bin_add_many(GST_BIN(pipeline), source, capsfilter, jpegenc, queue, filesink, nullptr);
    if (!gst_element_link_many(source, capsfilter, jpegenc, queue, filesink, nullptr))
    {
        ChipLogError(Camera, "Elements could not be linked.");
        gst_object_unref(pipeline);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    return pipeline;
}
} // namespace Snapshot

} // namespace GstreamerPipepline

CameraDevice::CameraDevice()
{
    // Set the CameraHALInterface in CameraAVStreamManager and CameraAVsettingsUserLevelManager.
    mCameraAVStreamManager.SetCameraDeviceHAL(this);
    mCameraAVSettingsUserLevelManager.SetCameraDeviceHAL(this);

    // Provider manager uses the Media controller to register WebRTC Transport with media controller for AV source data
    mWebRTCProviderManager.SetMediaController(&mMediaController);

    mPushAVTransportManager.SetMediaController(&mMediaController);

    // Set the CameraDevice interface in WebRTCManager
    mWebRTCProviderManager.SetCameraDevice(this);

    // Set the CameraDevice interface in ZoneManager
    mZoneManager.SetCameraDevice(this);
    mPushAVTransportManager.SetCameraDevice(this);
    mMediaController.SetCameraDevice(this);
}

CameraDevice::~CameraDevice()
{
    if (videoDeviceFd != -1)
    {
        close(videoDeviceFd);
    }
}

void CameraDevice::Init()
{
    InitializeCameraDevice();
    InitializeStreams();
    mWebRTCProviderManager.Init();
    mPushAVTransportManager.Init();
}

CameraError CameraDevice::InitializeCameraDevice()
{
    static bool gstreamerInitialized = false;

    if (!gstreamerInitialized)
    {
        gst_init(nullptr, nullptr);
        gstreamerInitialized = true;
    }

    ChipLogDetail(Camera, "InitializeCameraDevice: %s", mVideoDevicePath.c_str());

    videoDeviceFd = open(mVideoDevicePath.c_str(), O_RDWR);
    if (videoDeviceFd == -1)
    {
        ChipLogError(Camera, "Error opening video device: %s at %s", strerror(errno), mVideoDevicePath.c_str());
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
    const auto cameraType = GstreamerPipepline::detectCameraType(device);

    const GstreamerPipepline::Snapshot::SnapshotPipelineConfig config = {
        .device    = device,
        .width     = width,
        .height    = height,
        .quality   = quality,
        .framerate = framerate,
        .filename  = filename,
    };

    switch (cameraType)
    {
    case GstreamerPipepline::CameraType::kCsi: {
        ChipLogDetail(Camera, "Detected CSI camera: %s", device.c_str());
        return GstreamerPipepline::Snapshot::CreateSnapshotPipelineLibcamerasrc(config, error);
    }
    break;
    case GstreamerPipepline::CameraType::kUsb: {
        ChipLogDetail(Camera, "Detected USB camera: %s", device.c_str());
        return GstreamerPipepline::Snapshot::CreateSnapshotPipelineV4l2(config, error);
    }
    break;
    case GstreamerPipepline::CameraType::kFailure: {
        ChipLogError(Camera, "Unsupported camera type or device not found: %s", device.c_str());
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }
    }

    return nullptr; // Here to avoid compiler warnings, should never reach this point.
}

// Helper function to create a GStreamer pipeline that captures raw video frames from
// the camera, converts them to I420 format, encodes to H.264, and sends the encoded
// stream to the media controller via app sink.
GstElement * CameraDevice::CreateVideoPipeline(const std::string & device, int width, int height, int framerate,
                                               CameraError & error)
{
    GstElement * pipeline     = gst_pipeline_new("video-pipeline");
    GstElement * capsfilter1  = gst_element_factory_make("capsfilter", "filter1");
    GstElement * capsfilter2  = gst_element_factory_make("capsfilter", "filter2");
    GstElement * videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    GstElement * x264enc      = gst_element_factory_make("x264enc", "encoder");
    GstElement * appsink      = gst_element_factory_make("appsink", "appsink");
    GstElement * source       = nullptr;

    if (LinuxDeviceOptions::GetInstance().cameraTestVideosrc)
    {
        const int kBallAnimationPattern = 18;
        source                          = gst_element_factory_make("videotestsrc", "source");
        g_object_set(source, "pattern", kBallAnimationPattern, nullptr);
        ChipLogProgress(Camera, "Video piepline: using test video source");
    }
    else
    {
        source = gst_element_factory_make("v4l2src", "source");
        g_object_set(source, "device", device.c_str(), nullptr);
    }

    // Check for any nullptr among the created elements
    const std::vector<std::pair<GstElement *, const char *>> elements = {
        { pipeline, "pipeline" },         //
        { source, "source" },             //
        { capsfilter1, "filter1" },       //
        { videoconvert, "videoconvert" }, //
        { capsfilter2, "filter2" },       //
        { x264enc, "encoder" },           //
        { appsink, "appsink" }            //
    };
    const bool isElementFactoryMakeFailed = GstreamerPipepline::isGstElementsNull(elements);

    // If any element creation failed, log the error and unreference the elements
    if (isElementFactoryMakeFailed)
    {
        ChipLogError(Camera, "Not all elements could be created.");
        // Unreference the elements that were created
        GstreamerPipepline::unrefGstElements(pipeline, source, capsfilter1, videoconvert, capsfilter2, x264enc, appsink);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    // Camera caps request: RAW @ WxH @ fps
    GstCaps * caps1 = gst_caps_new_simple("video/x-raw", "width", G_TYPE_INT, width, "height", G_TYPE_INT, height, "framerate",
                                          GST_TYPE_FRACTION, framerate, 1, nullptr);
    g_object_set(capsfilter1, "caps", caps1, nullptr);
    gst_caps_unref(caps1);

    // Camera caps request: I420
    GstCaps * caps2 = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420", nullptr);
    g_object_set(capsfilter2, "caps", caps2, nullptr);
    gst_caps_unref(caps2);

    // Configure encoder for low‑latency and force IDR at start
    g_object_set(x264enc, "tune", 0, "speed-preset", 1, "key-int-max", framerate * 1, "insert-vui", TRUE, nullptr);

    // Configure appsink for receiving H.264 buffers data
    g_object_set(appsink, "emit-signals", TRUE, nullptr);

    // Build pipeline: v4l2src → capsfilter1 → videoconvert → capsfilter2 -> x264enc → appsink
    gst_bin_add_many(GST_BIN(pipeline), source, capsfilter1, videoconvert, capsfilter2, x264enc, appsink, nullptr);

    // Link the elements
    if (!gst_element_link_many(source, capsfilter1, videoconvert, capsfilter2, x264enc, appsink, nullptr))
    {
        ChipLogError(Camera, "CreateVideoPipeline: link failed");

        // The bin (pipeline) will unref all added elements automatically when you unref the bin.
        gst_object_unref(pipeline);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    return pipeline;
}

// Helper function to create a GStreamer pipeline
GstElement * CameraDevice::CreateAudioPipeline(const std::string & device, int channels, int sampleRate, int bitRate,
                                               CameraError & error)
{
    // Pipeline: source → capsfilter → audioconvert → audioresample → opusenc → appsink
    GstElement * pipeline = gst_pipeline_new("audio-pipeline");
    GstElement * source   = nullptr;

    if (LinuxDeviceOptions::GetInstance().cameraTestAudiosrc)
    {
        source = gst_element_factory_make("audiotestsrc", "source");
        g_object_set(source, "wave", 0, "is-live", TRUE, nullptr); // beep 0
        ChipLogProgress(Camera, "Audio piepline: using test audio source");
    }
    else
    {
        source = gst_element_factory_make("pulsesrc", "source");
        // g_object_set(source, "device", device.c_str(), nullptr);
    }

    GstElement * acaps   = gst_element_factory_make("capsfilter", "acaps");
    GstElement * aconv   = gst_element_factory_make("audioconvert", "aconv");
    GstElement * ares    = gst_element_factory_make("audioresample", "ares");
    GstElement * opusenc = gst_element_factory_make("opusenc", "opus");
    GstElement * appsink = gst_element_factory_make("appsink", "appsink");

    // Check creations (same helpers you already use for video)
    const std::vector<std::pair<GstElement *, const char *>> elements = {
        { pipeline, "pipeline" }, { source, "source" },   { acaps, "acaps" },     { aconv, "aconv" },
        { ares, "ares" },         { opusenc, "opusenc" }, { appsink, "appsink" },
    };

    const bool isElementFactoryMakeFailed = GstreamerPipepline::isGstElementsNull(elements);

    // If any element creation failed, log the error and unreference the elements
    if (isElementFactoryMakeFailed)
    {
        ChipLogError(Camera, "Not all elements could be created.");

        // Unreference the elements that were created
        GstreamerPipepline::unrefGstElements(pipeline, source, acaps, aconv, ares, opusenc, appsink);

        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    // Create GstCaps for the audio source
    GstCaps * caps = gst_caps_new_simple("audio/x-raw", "format", G_TYPE_STRING, "S16LE", "rate", G_TYPE_INT, sampleRate,
                                         "channels", G_TYPE_INT, channels, nullptr);
    g_object_set(acaps, "caps", caps, nullptr);
    gst_caps_unref(caps);

    // Match controller expectations: Opus @ 64 kbps (kAudioBitrate)
    g_object_set(opusenc, "bitrate", bitRate, "frame-size", AUDIO_FRAMESIZE, "inband-fec", FALSE, "dtx", FALSE, nullptr);

    // Emit samples to your appsink callback (DistributeAudio → packetizer)
    g_object_set(appsink, "emit-signals", TRUE, nullptr);
    g_object_set(source, "do-timestamp", TRUE, nullptr);

    // Build and link
    gst_bin_add_many(GST_BIN(pipeline), source, acaps, aconv, ares, opusenc, appsink, nullptr);
    if (!gst_element_link_many(source, acaps, aconv, ares, opusenc, appsink, nullptr))
    {
        ChipLogError(Camera, "CreateAudioPipeline: link failed");
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    return pipeline;
}

// Helper function to create a GStreamer pipeline for audio playback
GstElement * CameraDevice::CreateAudioPlaybackPipeline(CameraError & error)
{
    GstElement * pipeline        = gst_pipeline_new("audio-playback-pipeline");
    GstElement * udpsrc          = gst_element_factory_make("udpsrc", "audio_udpsrc");
    GstElement * rtpjitterbuffer = gst_element_factory_make("rtpjitterbuffer", "rtp_jitter_buffer");
    GstElement * rtpopusdepay    = gst_element_factory_make("rtpopusdepay", "rtp_opus_depay");
    GstElement * opusdec         = gst_element_factory_make("opusdec", "opus_dec");
    GstElement * audioconvert    = gst_element_factory_make("audioconvert", "audio_convert");
    GstElement * audioresample   = gst_element_factory_make("audioresample", "audio_resample");
    GstElement * capsfilter      = gst_element_factory_make("capsfilter", "audio_caps");
    GstElement * queue           = gst_element_factory_make("queue", "audio_queue");
    GstElement * autoaudiosink   = gst_element_factory_make("autoaudiosink", "audio_sink");

    const std::vector<std::pair<GstElement *, const char *>> elements = {
        { pipeline, "pipeline" },           { udpsrc, "udpsrc" },         { rtpjitterbuffer, "rtpjitterbuffer" },
        { rtpopusdepay, "rtpopusdepay" },   { opusdec, "opusdec" },       { audioconvert, "audioconvert" },
        { audioresample, "audioresample" }, { capsfilter, "capsfilter" }, { queue, "queue" },
        { autoaudiosink, "autoaudiosink" }
    };

    if (GstreamerPipepline::isGstElementsNull(elements))
    {
        ChipLogError(Camera, "Not all audio playback pipeline elements could be created.");
        GstreamerPipepline::unrefGstElements(pipeline, udpsrc, rtpjitterbuffer, rtpopusdepay, opusdec, audioconvert, audioresample,
                                             capsfilter, queue, autoaudiosink);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    g_object_set(udpsrc, "port", 6001, nullptr);
    GstCaps * udpsrc_caps =
        gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING, "audio", "encoding-name", G_TYPE_STRING, "OPUS", "payload",
                            G_TYPE_INT, 111, "clock-rate", G_TYPE_INT, 48000, "channel", G_TYPE_INT, 1, nullptr);
    g_object_set(udpsrc, "caps", udpsrc_caps, nullptr);
    gst_caps_unref(udpsrc_caps);

    // Configure rtpjitterbuffer
    // latency is set high (2.5s) because too much underflow, clock-skew issues are observed with the piepline is receiving audio
    // stream from libdatachannel.
    g_object_set(rtpjitterbuffer, "latency", 2500, nullptr);

    // Configure capsfilter for autoaudiosink
    GstCaps * sink_caps = gst_caps_new_simple("audio/x-raw", "format", G_TYPE_STRING, "S16LE", "layout", G_TYPE_STRING,
                                              "interleaved", "channels", G_TYPE_INT, 1, "rate", G_TYPE_INT, 48000, nullptr);
    g_object_set(capsfilter, "caps", sink_caps, nullptr);
    gst_caps_unref(sink_caps);
    g_object_set(autoaudiosink, "sync", FALSE, nullptr);

    gst_bin_add_many(GST_BIN(pipeline), udpsrc, rtpjitterbuffer, rtpopusdepay, opusdec, audioconvert, audioresample, capsfilter,
                     queue, autoaudiosink, nullptr);

    if (!gst_element_link_many(udpsrc, rtpjitterbuffer, rtpopusdepay, opusdec, audioconvert, audioresample, capsfilter, queue,
                               autoaudiosink, nullptr))
    {
        ChipLogError(Camera, "Audio playback pipeline elements could not be linked.");
        gst_object_unref(pipeline);
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

// Find the closest allocated snapshot stream with resolution >= requested, or closest possible
bool CameraDevice::MatchClosestSnapshotParams(const VideoResolutionStruct & requested, VideoResolutionStruct & matchedResolution,
                                              ImageCodecEnum & matchedCodec)
{
    int64_t requestedPixels = static_cast<int64_t>(requested.width) * requested.height;
    int64_t bestDiff        = std::numeric_limits<int64_t>::max();
    int64_t bestGEQDiff     = std::numeric_limits<int64_t>::max();

    const SnapshotStream * bestStream    = nullptr;
    const SnapshotStream * bestGEQStream = nullptr;

    for (const auto & stream : mSnapshotStreams)
    {
        int64_t streamPixels = static_cast<int64_t>(stream.snapshotStreamParams.minResolution.width) *
            stream.snapshotStreamParams.minResolution.height;
        int64_t diff    = streamPixels - requestedPixels;
        int64_t absDiff = std::abs(diff);

        // Candidate 1: First stream with resolution >= requested
        if (diff >= 0 && diff < bestGEQDiff)
        {
            bestGEQDiff   = diff;
            bestGEQStream = &stream;
        }

        // Candidate 2: Closest stream (absolute difference)
        if (absDiff < bestDiff)
        {
            bestDiff   = absDiff;
            bestStream = &stream;
        }
    }

    const SnapshotStream * chosen = bestGEQStream ? bestGEQStream : bestStream;
    if (chosen)
    {
        matchedResolution = chosen->snapshotStreamParams.minResolution;
        matchedCodec      = chosen->snapshotStreamParams.imageCodec;
        return true;
    }
    return false;
}

CameraError CameraDevice::CaptureSnapshot(const chip::app::DataModel::Nullable<uint16_t> streamID,
                                          const VideoResolutionStruct & resolution, ImageSnapshot & outImageSnapshot)
{
    VideoResolutionStruct matchedRes;
    ImageCodecEnum matchedCodec;

    if (streamID.IsNull())
    {
        if (!MatchClosestSnapshotParams(resolution, matchedRes, matchedCodec))
        {
            ChipLogError(Camera, "No matching snapshot stream found for requested resolution %ux%u", resolution.width,
                         resolution.height);
            return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
        }
    }
    else
    {
        uint16_t streamId = streamID.Value();
        auto it           = std::find_if(mSnapshotStreams.begin(), mSnapshotStreams.end(), [streamId](const SnapshotStream & s) {
            return s.snapshotStreamParams.snapshotStreamID == streamId;
        });
        if (it == mSnapshotStreams.end())
        {
            ChipLogError(Camera, "Snapshot stream not found for stream ID %u", streamId);
            return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
        }
        matchedRes   = it->snapshotStreamParams.minResolution;
        matchedCodec = it->snapshotStreamParams.imageCodec;
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

    outImageSnapshot.imageRes   = matchedRes;
    outImageSnapshot.imageCodec = matchedCodec;

    return CameraError::SUCCESS;
}

CameraError CameraDevice::StartVideoStream(const VideoStreamStruct & allocatedStream)
{
    uint16_t streamID = allocatedStream.videoStreamID;
    auto it           = std::find_if(mVideoStreams.begin(), mVideoStreams.end(),
                                     [streamID](const VideoStream & s) { return s.videoStreamParams.videoStreamID == streamID; });

    if (it == mVideoStreams.end())
    {
        return CameraError::ERROR_VIDEO_STREAM_START_FAILED;
    }

    // Create Gstreamer video pipeline using the final allocated stream parameters
    CameraError error          = CameraError::SUCCESS;
    GstElement * videoPipeline = CreateVideoPipeline(mVideoDevicePath, allocatedStream.minResolution.width,
                                                     allocatedStream.minResolution.height, allocatedStream.minFrameRate, error);
    if (videoPipeline == nullptr)
    {
        ChipLogError(Camera, "Failed to create video pipeline.");
        it->videoContext = nullptr;
        return error;
    }

    // Get the appsink and set up callback
    GstElement * appsink = gst_bin_get_by_name(GST_BIN(videoPipeline), "appsink");
    if (appsink)
    {
        VideoAppSinkContext * context = new VideoAppSinkContext{ this, streamID };
        GstAppSinkCallbacks callbacks = { nullptr, nullptr, OnNewVideoSampleFromAppSink };
        gst_app_sink_set_callbacks(GST_APP_SINK(appsink), &callbacks, context, DestroyVideoAppSinkContext);
        gst_object_unref(appsink);
    }

    ChipLogProgress(Camera, "Starting video stream (id=%u): %u×%u @ %ufps", streamID, allocatedStream.minResolution.width,
                    allocatedStream.minResolution.height, allocatedStream.minFrameRate);

    // Start the pipeline
    ChipLogProgress(Camera, "Requesting PLAYING …");
    GstStateChangeReturn result = gst_element_set_state(videoPipeline, GST_STATE_PLAYING);
    if (result == GST_STATE_CHANGE_FAILURE)
    {
        // Get error message from GStreamer bus
        GstBus * bus = gst_element_get_bus(videoPipeline);
        if (bus)
        {
            GstMessage * msg = gst_bus_pop_filtered(bus, (GstMessageType) (GST_MESSAGE_ERROR | GST_MESSAGE_WARNING));
            if (msg)
            {
                GError * err       = nullptr;
                gchar * debug_info = nullptr;

                if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR)
                {
                    gst_message_parse_error(msg, &err, &debug_info);
                    ChipLogError(Camera, "GStreamer Error: %s", err ? err->message : "unknown");
                    if (debug_info)
                    {
                        ChipLogError(Camera, "Debug info: %s", debug_info);
                    }
                }
                else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_WARNING)
                {
                    gst_message_parse_warning(msg, &err, &debug_info);
                    ChipLogError(Camera, "GStreamer Warning: %s", err ? err->message : "unknown");
                    if (debug_info)
                    {
                        ChipLogError(Camera, "Debug info: %s", debug_info);
                    }
                }

                g_error_free(err);
                g_free(debug_info);
                gst_message_unref(msg);
            }
            gst_object_unref(bus);
        }

        ChipLogError(Camera, "Failed to start video pipeline.");
        gst_object_unref(videoPipeline);
        it->videoContext = nullptr;
        return CameraError::ERROR_VIDEO_STREAM_START_FAILED;
    }

    // Wait for the pipeline to reach the PLAYING state
    GstState state;
    gst_element_get_state(videoPipeline, &state, nullptr, VIDEO_PIPELINE_PLAY_TIMEOUT * GST_SECOND);
    if (state != GST_STATE_PLAYING)
    {
        ChipLogError(Camera, "Video pipeline did not reach PLAYING state.");
        gst_element_set_state(videoPipeline, GST_STATE_NULL);
        gst_object_unref(videoPipeline);
        it->videoContext = nullptr;
        return CameraError::ERROR_VIDEO_STREAM_START_FAILED;
    }

    // Store in stream context
    it->videoContext = videoPipeline;

    ChipLogProgress(Camera, "Video is PLAYING …");

    return CameraError::SUCCESS;
}

// Stop video stream
CameraError CameraDevice::StopVideoStream(uint16_t streamID)
{
    auto it = std::find_if(mVideoStreams.begin(), mVideoStreams.end(),
                           [streamID](const VideoStream & s) { return s.videoStreamParams.videoStreamID == streamID; });

    if (it == mVideoStreams.end())
    {
        return CameraError::ERROR_VIDEO_STREAM_STOP_FAILED;
    }

    GstElement * videoPipeline = reinterpret_cast<GstElement *>(it->videoContext);
    if (videoPipeline != nullptr)
    {
        GstStateChangeReturn result = gst_element_set_state(videoPipeline, GST_STATE_NULL);

        // Always clean up, regardless of state change result
        gst_object_unref(videoPipeline);
        it->videoContext = nullptr;

        if (result == GST_STATE_CHANGE_FAILURE)
        {
            return CameraError::ERROR_VIDEO_STREAM_STOP_FAILED;
        }
    }

    return CameraError::SUCCESS;
}

// Start audio stream
CameraError CameraDevice::StartAudioStream(uint16_t streamID)
{
    auto it = std::find_if(mAudioStreams.begin(), mAudioStreams.end(),
                           [streamID](const AudioStream & s) { return s.audioStreamParams.audioStreamID == streamID; });

    if (it == mAudioStreams.end())
    {
        ChipLogError(Camera, "Audio streamID : %u not found", streamID);
        return CameraError::ERROR_AUDIO_STREAM_START_FAILED;
    }

    int channels   = it->audioStreamParams.channelCount;
    int sampleRate = static_cast<int>(it->audioStreamParams.sampleRate);
    int bitRate    = static_cast<int>(it->audioStreamParams.bitRate);

    // Create Gstreamer audio pipeline
    CameraError error          = CameraError::SUCCESS;
    GstElement * audioPipeline = CreateAudioPipeline("/dev/audio0", channels, sampleRate, bitRate, error);
    if (audioPipeline == nullptr)
    {
        ChipLogError(Camera, "Failed to create audio pipeline.");
        it->audioContext = nullptr;
        return CameraError::ERROR_AUDIO_STREAM_START_FAILED;
    }

    // Get the appsink and set up callback
    GstElement * appsink = gst_bin_get_by_name(GST_BIN(audioPipeline), "appsink");
    if (appsink)
    {
        AudioAppSinkContext * context = new AudioAppSinkContext{ this, streamID };
        GstAppSinkCallbacks callbacks = { nullptr, nullptr, OnNewAudioSampleFromAppSink };
        gst_app_sink_set_callbacks(GST_APP_SINK(appsink), &callbacks, context, DestroyAudioAppSinkContext);
        gst_object_unref(appsink);
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
    else
    {
        ChipLogProgress(Camera, "Audio Pipeline reached playing state");
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

    // Store in stream context
    it->audioContext = audioPipeline;

    // Start the audio playback pipeline
    if (LinuxDeviceOptions::GetInstance().cameraAudioPlayback)
    {
        CameraError playbackError = StartAudioPlaybackStream();
        if (playbackError != CameraError::SUCCESS)
        {
            ChipLogError(Camera, "Failed to start audio playback pipeline for stream ID: %u. Error: %d", streamID,
                         static_cast<int>(playbackError));
        }
    }

    return CameraError::SUCCESS;
}

// Stop audio stream
CameraError CameraDevice::StopAudioStream(uint16_t streamID)
{
    auto it = std::find_if(mAudioStreams.begin(), mAudioStreams.end(),
                           [streamID](const AudioStream & s) { return s.audioStreamParams.audioStreamID == streamID; });

    if (it == mAudioStreams.end())
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

    // Stop the audio playback pipeline
    if (LinuxDeviceOptions::GetInstance().cameraAudioPlayback)
    {
        CameraError playbackError = StopAudioPlaybackStream();
        if (playbackError != CameraError::SUCCESS)
        {
            ChipLogError(Camera, "Failed to stop audio playback pipeline for stream ID: %u. Error: %d", streamID,
                         static_cast<int>(playbackError));
        }
    }

    return CameraError::SUCCESS;
}

CameraError CameraDevice::StartAudioPlaybackStream()
{

    if (mAudioPlaybackPipeline != nullptr)
    {
        ChipLogError(Camera, "Audio playback pipeline already exists. Stop it before starting a new one.");
        return CameraError::ERROR_AUDIO_STREAM_START_FAILED;
    }

    CameraError error      = CameraError::SUCCESS;
    mAudioPlaybackPipeline = CreateAudioPlaybackPipeline(error);
    if (mAudioPlaybackPipeline == nullptr)
    {
        ChipLogError(Camera, "Failed to create audio playback pipeline.");
        return error;
    }

    GstStateChangeReturn result = gst_element_set_state(mAudioPlaybackPipeline, GST_STATE_PLAYING);
    if (result == GST_STATE_CHANGE_FAILURE)
    {
        ChipLogError(Camera, "Failed to start audio playback pipeline.");
        gst_object_unref(mAudioPlaybackPipeline);
        mAudioPlaybackPipeline = nullptr;
        return CameraError::ERROR_AUDIO_STREAM_START_FAILED;
    }

    ChipLogProgress(Camera, "Audio playback pipeline started");
    return CameraError::SUCCESS;
}

CameraError CameraDevice::StopAudioPlaybackStream()
{
    if (mAudioPlaybackPipeline == nullptr)
    {
        ChipLogDetail(Camera, "Audio playback pipeline is not running or already stopped.");
        return CameraError::SUCCESS;
    }

    ChipLogProgress(Camera, "Stopping audio playback pipeline");
    GstStateChangeReturn result = gst_element_set_state(mAudioPlaybackPipeline, GST_STATE_NULL);
    gst_object_unref(mAudioPlaybackPipeline);
    mAudioPlaybackPipeline = nullptr;

    if (result == GST_STATE_CHANGE_FAILURE)
    {
        ChipLogError(Camera, "Failed to stop audio playback pipeline.");
        return CameraError::ERROR_AUDIO_STREAM_STOP_FAILED;
    }

    ChipLogProgress(Camera, "Audio playback pipeline stopped.");
    return CameraError::SUCCESS;
}

// Allocate snapshot stream
CameraError CameraDevice::AllocateSnapshotStream(const CameraAVStreamManagementDelegate::SnapshotStreamAllocateArgs & args,
                                                 uint16_t & outStreamID)
{

    if (AddSnapshotStream(args, outStreamID))
    {
        auto it = std::find_if(mSnapshotStreams.begin(), mSnapshotStreams.end(), [outStreamID](const SnapshotStream & s) {
            return s.snapshotStreamParams.snapshotStreamID == outStreamID;
        });
        if (it == mSnapshotStreams.end())
        {
            ChipLogError(Camera, "Snapshot stream with ID %u not found", outStreamID);
            return CameraError::ERROR_RESOURCE_EXHAUSTED;
        }
        it->isAllocated = true;
        ChipLogProgress(Camera, "Allocated snapshot stream with ID: %u", outStreamID);
        return CameraError::SUCCESS;
    }
    return CameraError::ERROR_RESOURCE_EXHAUSTED;
}

// Start snapshot stream
CameraError CameraDevice::StartSnapshotStream(uint16_t streamID)
{
    auto it = std::find_if(mSnapshotStreams.begin(), mSnapshotStreams.end(),
                           [streamID](const SnapshotStream & s) { return s.snapshotStreamParams.snapshotStreamID == streamID; });
    if (it == mSnapshotStreams.end())
    {
        ChipLogError(Camera, "Snapshot streamID : %u not found", streamID);
        return CameraError::ERROR_SNAPSHOT_STREAM_START_FAILED;
    }

    // Create the GStreamer pipeline
    CameraError error             = CameraError::SUCCESS;
    GstElement * snapshotPipeline = CreateSnapshotPipeline(
        mVideoDevicePath, it->snapshotStreamParams.minResolution.width, it->snapshotStreamParams.minResolution.height,
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
    auto it = std::find_if(mSnapshotStreams.begin(), mSnapshotStreams.end(),
                           [streamID](const SnapshotStream & s) { return s.snapshotStreamParams.snapshotStreamID == streamID; });
    if (it == mSnapshotStreams.end())
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

bool CameraDevice::GetCameraSupportsHDR()
{
    return true;
}

bool CameraDevice::GetCameraSupportsNightVision()
{
    return true;
}

bool CameraDevice::GetNightVisionUsesInfrared()
{
    return false;
}

bool CameraDevice::GetCameraSupportsWatermark()
{
    return true;
}

bool CameraDevice::GetCameraSupportsOSD()
{
    return true;
}

bool CameraDevice::GetCameraSupportsSoftPrivacy()
{
    return true;
}

bool CameraDevice::GetCameraSupportsImageControl()
{
    return true;
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
    static std::array<AudioCodecEnum, 2> audioCodecs   = { AudioCodecEnum::kOpus, AudioCodecEnum::kAacLc };
    static std::array<uint32_t, 2> sampleRates         = { 48000, 32000 }; // Sample rates in Hz
    static std::array<uint8_t, 2> bitDepths            = { 24, 32 };
    static AudioCapabilitiesStruct speakerCapabilities = { kSpeakerMaxChannelCount, chip::Span<AudioCodecEnum>(audioCodecs),
                                                           chip::Span<uint32_t>(sampleRates), chip::Span<uint8_t>(bitDepths) };
    return speakerCapabilities;
}

std::vector<SnapshotCapabilitiesStruct> & CameraDevice::GetSnapshotCapabilities()
{
    static std::vector<SnapshotCapabilitiesStruct> snapshotCapabilities = {
        { { kMinResolutionWidth, kMinResolutionHeight },
          kSnapshotStreamFrameRate,
          ImageCodecEnum::kJpeg,
          false,
          chip::MakeOptional(static_cast<bool>(false)) },
        { { k720pResolutionWidth, k720pResolutionHeight },
          kSnapshotStreamFrameRate,
          ImageCodecEnum::kJpeg,
          true,
          chip::MakeOptional(static_cast<bool>(true)) },
    };
    return snapshotCapabilities;
}

CameraError CameraDevice::SetNightVision(TriStateAutoEnum nightVision)
{
    mNightVision = nightVision;

    return CameraError::SUCCESS;
}

uint32_t CameraDevice::GetMaxNetworkBandwidth()
{
    return kMaxNetworkBandwidthbps;
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

CameraError CameraDevice::SetHardPrivacyMode(bool hardPrivacyMode)
{
    ChipLogProgress(Camera, "SetHardPrivacyMode: Setting hard privacy mode to %s", hardPrivacyMode ? "true" : "false");
    mHardPrivacyModeOn = hardPrivacyMode;

    return CameraError::SUCCESS;
}

CameraError CameraDevice::SetStreamUsagePriorities(std::vector<StreamUsageEnum> streamUsagePriorities)
{
    mStreamUsagePriorities = streamUsagePriorities;

    return CameraError::SUCCESS;
}

std::vector<StreamUsageEnum> & CameraDevice::GetSupportedStreamUsages()
{
    static std::vector<StreamUsageEnum> supportedStreamUsage = { StreamUsageEnum::kLiveView, StreamUsageEnum::kRecording };
    return supportedStreamUsage;
}

CameraError CameraDevice::SetViewport(const chip::app::Clusters::Globals::Structs::ViewportStruct::Type & viewPort)
{
    mViewport = viewPort;

    return CameraError::SUCCESS;
}

CameraError CameraDevice::SetViewport(VideoStream & stream,
                                      const chip::app::Clusters::Globals::Structs::ViewportStruct::Type & viewport)
{
    ChipLogDetail(Camera, "Setting per stream viewport for stream %d.", stream.videoStreamParams.videoStreamID);
    ChipLogDetail(Camera, "New viewport. x1=%d, x2=%d, y1=%d, y2=%d.", viewport.x1, viewport.x2, viewport.y1, viewport.y2);
    stream.viewport = viewport;
    return CameraError::SUCCESS;
}

CameraError CameraDevice::SetSoftRecordingPrivacyModeEnabled(bool softRecordingPrivacyMode)
{
    mSoftRecordingPrivacyModeEnabled = softRecordingPrivacyMode;

    // Notify the PushAVManager about change
    mPushAVTransportManager.RecordingStreamPrivacyModeChanged(softRecordingPrivacyMode);

    return CameraError::SUCCESS;
}

CameraError CameraDevice::SetSoftLivestreamPrivacyModeEnabled(bool softLivestreamPrivacyMode)
{
    mSoftLivestreamPrivacyModeEnabled = softLivestreamPrivacyMode;

    // Notify WebRTCProviderManager about change
    mWebRTCProviderManager.LiveStreamPrivacyModeChanged(softLivestreamPrivacyMode);

    return CameraError::SUCCESS;
}

// Mute/Unmute speaker.
CameraError CameraDevice::SetSpeakerMuted(bool muteSpeaker)
{
    mSpeakerMuted = muteSpeaker;

    return CameraError::SUCCESS;
}

// Set speaker volume level.
CameraError CameraDevice::SetSpeakerVolume(uint8_t speakerVol)
{
    mSpeakerVol = speakerVol;

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

// Set image rotation attributes
CameraError CameraDevice::SetImageRotation(uint16_t imageRotation)
{
    mImageRotation = imageRotation;

    return CameraError::SUCCESS;
}

CameraError CameraDevice::SetImageFlipHorizontal(bool imageFlipHorizontal)
{
    mImageFlipHorizontal = imageFlipHorizontal;

    return CameraError::SUCCESS;
}

CameraError CameraDevice::SetImageFlipVertical(bool imageFlipVertical)
{
    mImageFlipVertical = imageFlipVertical;

    return CameraError::SUCCESS;
}

CameraError CameraDevice::SetLocalVideoRecordingEnabled(bool localVideoRecordingEnabled)
{
    mLocalVideoRecordingEnabled = localVideoRecordingEnabled;

    return CameraError::SUCCESS;
}

CameraError CameraDevice::SetLocalSnapshotRecordingEnabled(bool localSnapshotRecordingEnabled)
{
    mLocalSnapshotRecordingEnabled = localSnapshotRecordingEnabled;

    return CameraError::SUCCESS;
}

CameraError CameraDevice::SetStatusLightEnabled(bool statusLightEnabled)
{
    mStatusLightEnabled = statusLightEnabled;

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

// Set the PTZ values as received
CameraError CameraDevice::SetPhysicalPTZ(chip::Optional<int16_t> aPan, chip::Optional<int16_t> aTilt, chip::Optional<uint8_t> aZoom)
{
    if (aPan.HasValue())
    {
        SetPan(aPan.Value());
    }

    if (aTilt.HasValue())
    {
        SetTilt(aTilt.Value());
    }

    if (aZoom.HasValue())
    {
        SetZoom(aZoom.Value());
    }

    return CameraError::SUCCESS;
}

CameraError CameraDevice::SetDetectionSensitivity(uint8_t aSensitivity)
{
    mDetectionSensitivity = aSensitivity;
    return CameraError::SUCCESS;
}

CameraError CameraDevice::CreateZoneTrigger(const ZoneTriggerControlStruct & zoneTrigger)
{

    return CameraError::SUCCESS;
}

CameraError CameraDevice::UpdateZoneTrigger(const ZoneTriggerControlStruct & zoneTrigger)
{

    return CameraError::SUCCESS;
}

CameraError CameraDevice::RemoveZoneTrigger(const uint16_t zoneId)
{

    return CameraError::SUCCESS;
}

void CameraDevice::HandleSimulatedZoneTriggeredEvent(uint16_t zoneId)
{
    mZoneManager.OnZoneTriggeredEvent(zoneId, ZoneEventTriggeredReasonEnum::kMotion);
    mPushAVTransportManager.HandleZoneTrigger(zoneId);
}

void CameraDevice::HandleSimulatedZoneStoppedEvent(uint16_t zoneId)
{
    mZoneManager.OnZoneStoppedEvent(zoneId, ZoneEventStoppedReasonEnum::kActionStopped);
    // Note: PushAVTransportManager doesn't need zone stopped event currently
}

void CameraDevice::InitializeVideoStreams()
{
    // Create a video stream with a max resolution of 720p and max frame rate of
    // 60 fps
    VideoStream videoStream1 = { { 1 /* Id */,
                                   StreamUsageEnum::kLiveView /* StreamUsage */,
                                   VideoCodecEnum::kH264,
                                   kMinVideoFrameRate /* MinFrameRate */,
                                   k60fpsVideoFrameRate /* MaxFrameRate */,
                                   { kMinResolutionWidth, kMinResolutionHeight } /* MinResolution */,
                                   { k720pResolutionWidth, k720pResolutionHeight } /* MaxResolution */,
                                   kMinBitRateBps /* MinBitRate */,
                                   kMaxBitRateBps /* MaxBitRate */,
                                   kKeyFrameIntervalMsec /* KeyFrameInterval */,
                                   chip::MakeOptional(static_cast<bool>(false)) /* WMark */,
                                   chip::MakeOptional(static_cast<bool>(false)) /* OSD */,
                                   0 /* RefCount */ },
                                 false,
                                 { mViewport.x1, mViewport.y1, mViewport.x2, mViewport.y2 },
                                 nullptr };
    mVideoStreams.push_back(videoStream1);

    // Create a video stream with a min framerate of 60 fps and min resolution
    // of 720p
    VideoStream videoStream2 = { { 2 /* Id */,
                                   StreamUsageEnum::kLiveView /* StreamUsage */,
                                   VideoCodecEnum::kH264,
                                   k60fpsVideoFrameRate /* MinFrameRate */,
                                   kMaxVideoFrameRate /* MaxFrameRate */,
                                   { k720pResolutionWidth, k720pResolutionHeight } /* MinResolution */,
                                   { kMaxResolutionWidth, kMaxResolutionHeight } /* MaxResolution */,
                                   kMinBitRateBps /* MinBitRate */,
                                   kMaxBitRateBps /* MaxBitRate */,
                                   kKeyFrameIntervalMsec /* KeyFrameInterval */,
                                   chip::MakeOptional(static_cast<bool>(false)) /* WMark */,
                                   chip::MakeOptional(static_cast<bool>(false)) /* OSD */,
                                   0 /* RefCount */ },
                                 false,
                                 { mViewport.x1, mViewport.y1, mViewport.x2, mViewport.y2 },
                                 nullptr };

    mVideoStreams.push_back(videoStream2);

    // Create a video stream for the full range(fps, resolution, bitrate) supported by the camera.
    VideoStream videoStream3 = { { 3 /* Id */,
                                   StreamUsageEnum::kLiveView /* StreamUsage */,
                                   VideoCodecEnum::kH264,
                                   kMinVideoFrameRate /* MinFrameRate */,
                                   kMaxVideoFrameRate /* MaxFrameRate */,
                                   { kMinResolutionWidth, kMinResolutionHeight } /* MinResolution */,
                                   { kMaxResolutionWidth, kMaxResolutionHeight } /* MaxResolution */,
                                   kMinBitRateBps /* MinBitRate */,
                                   kMaxBitRateBps /* MaxBitRate */,
                                   kKeyFrameIntervalMsec /* KeyFrameInterval */,
                                   chip::MakeOptional(static_cast<bool>(false)) /* WMark */,
                                   chip::MakeOptional(static_cast<bool>(false)) /* OSD */,
                                   0 /* RefCount */ },
                                 false,
                                 { mViewport.x1, mViewport.y1, mViewport.x2, mViewport.y2 },
                                 nullptr };

    mVideoStreams.push_back(videoStream3);
}

void CameraDevice::InitializeAudioStreams()
{
    // Mono stream
    AudioStream monoStream = { { 1 /* Id */, StreamUsageEnum::kLiveView, AudioCodecEnum::kOpus, 1 /* ChannelCount: Mono */,
                                 48000 /* SampleRate */, 20000 /* BitRate */, 24 /* BitDepth */, 0 /* RefCount */ },
                               false,
                               nullptr };
    mAudioStreams.push_back(monoStream);

    // Stereo stream
    AudioStream stereoStream = { { 2 /* Id */, StreamUsageEnum::kLiveView, AudioCodecEnum::kOpus, 2 /* ChannelCount: Stereo */,
                                   48000 /* SampleRate */, 32000 /* BitRate */, 24 /* BitDepth */, 0 /* RefCount */ },
                                 false,
                                 nullptr };
    mAudioStreams.push_back(stereoStream);

    // Max channel count stream (from spec constant)
    AudioStream maxChannelStream = { { 3 /* Id */, StreamUsageEnum::kLiveView, AudioCodecEnum::kOpus,
                                       kMicrophoneMaxChannelCount /* Max from Spec */, 48000 /* SampleRate */, 64000 /* BitRate */,
                                       24 /* BitDepth */, 0 /* RefCount */ },
                                     false,
                                     nullptr };
    mAudioStreams.push_back(maxChannelStream);
}

void CameraDevice::InitializeSnapshotStreams()
{
    // Create single snapshot stream with typical supported parameters
    uint16_t streamId = kInvalidStreamID;
    AddSnapshotStream({ ImageCodecEnum::kJpeg,
                        kSnapshotStreamFrameRate /* FrameRate */,
                        { kMinResolutionWidth, kMinResolutionHeight } /* MinResolution*/,
                        { kMaxResolutionWidth, kMaxResolutionHeight } /* MaxResolution */,
                        90 /* Quality */ },
                      streamId);
}

bool CameraDevice::AddSnapshotStream(
    const CameraAVStreamManagementDelegate::SnapshotStreamAllocateArgs & snapshotStreamAllocateArgs, uint16_t & outStreamID)
{
    constexpr uint16_t kMaxSnapshotStreams = std::numeric_limits<uint16_t>::max();

    if (mSnapshotStreams.size() >= kMaxSnapshotStreams)
    {
        ChipLogError(Camera, "Maximum number of snapshot streams reached. Cannot a allocate new one");
        return false;
    }

    uint16_t streamId = 0;
    // Fetch a new stream ID if the passed ID is kInvalidStreamID, otherwise use
    // the ID that was passed in. A valid streamID would be passed in when the
    // stream list is being constructed from the persisted list of allocated
    // streams that was loaded at Init()
    if (outStreamID == kInvalidStreamID)
    {
        for (const auto & s : mSnapshotStreams)
        {
            // Find the highest existing stream ID.
            if (s.snapshotStreamParams.snapshotStreamID > streamId)
            {
                streamId = s.snapshotStreamParams.snapshotStreamID;
            }
        }

        // Find a unique stream id, starting from the last used one above, incrementing and wrapping at 65535.
        for (uint16_t attempts = 0; attempts < kMaxSnapshotStreams; ++attempts)
        {
            auto found = std::find_if(mSnapshotStreams.begin(), mSnapshotStreams.end(), [streamId](const SnapshotStream & s) {
                return s.snapshotStreamParams.snapshotStreamID == streamId;
            });
            if (found == mSnapshotStreams.end())
            {
                break;
            }
            if (attempts == kMaxSnapshotStreams - 1)
            {
                ChipLogError(Camera, "No available slot for stream allocation");
                return false;
            }
            streamId = static_cast<uint16_t>((streamId + 1) % kMaxSnapshotStreams); // Wraps to 0 after max-1
        }

        outStreamID = streamId;
    }
    else
    {
        // Have a sanity check that the passed streamID does not already exist
        // in the list
        auto found = std::find_if(mSnapshotStreams.begin(), mSnapshotStreams.end(), [outStreamID](const SnapshotStream & s) {
            return s.snapshotStreamParams.snapshotStreamID == outStreamID;
        });

        if (found == mSnapshotStreams.end())
        {
            streamId = outStreamID;
        }
        else
        {
            ChipLogError(Camera, "StreamID %d already exists in the available snapshot stream list", outStreamID);
            return false;
        }
    }

    SnapshotStream snapshotStream = { { streamId, snapshotStreamAllocateArgs.imageCodec, snapshotStreamAllocateArgs.maxFrameRate,
                                        snapshotStreamAllocateArgs.minResolution, snapshotStreamAllocateArgs.maxResolution,
                                        snapshotStreamAllocateArgs.quality, 0 /* RefCount */ },
                                      false,
                                      nullptr };

    mSnapshotStreams.push_back(snapshotStream);
    return true;
}

ChimeDelegate & CameraDevice::GetChimeDelegate()
{
    return mChimeManager;
}

WebRTCTransportProvider::Delegate & CameraDevice::GetWebRTCProviderDelegate()
{
    return mWebRTCProviderManager;
}

void CameraDevice::SetWebRTCTransportProvider(WebRTCTransportProvider::WebRTCTransportProviderCluster * provider)
{
    mWebRTCProviderManager.SetWebRTCTransportProvider(provider);
}

PushAvStreamTransportDelegate & CameraDevice::GetPushAVTransportDelegate()
{
    return mPushAVTransportManager;
}

CameraAVStreamManagementDelegate & CameraDevice::GetCameraAVStreamMgmtDelegate()
{
    return mCameraAVStreamManager;
}

CameraAVStreamController & CameraDevice::GetCameraAVStreamMgmtController()
{
    return mCameraAVStreamManager;
}

CameraAvSettingsUserLevelManagementDelegate & CameraDevice::GetCameraAVSettingsUserLevelMgmtDelegate()
{
    return mCameraAVSettingsUserLevelManager;
}

ZoneManagement::Delegate & CameraDevice::GetZoneManagementDelegate()
{
    return mZoneManager;
}

MediaController & CameraDevice::GetMediaController()
{
    return mMediaController;
}

size_t CameraDevice::GetPreRollBufferSize()
{
    return kMaxContentBufferSizeBytes;
}

int64_t CameraDevice::GetMinKeyframeIntervalMs()
{
    return kKeyFrameIntervalMsec;
}
