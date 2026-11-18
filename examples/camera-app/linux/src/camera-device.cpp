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
#include <algorithm>
#include <app/clusters/zone-management-server/zone-geometry.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fcntl.h> // For file descriptor operations
#include <filesystem>
#include <fstream>
#include <gst/app/gstappsink.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>
#include <limits.h>          // For PATH_MAX
#include <linux/videodev2.h> // For V4L2 definitions
#include <memory>
#include <platform/CHIPDeviceLayer.h>
#include <sys/ioctl.h>

// Timeout for video pipeline to go to playing state.
#define VIDEO_PIPELINE_PLAY_TIMEOUT 5
// Framesize for audio pipeline
#define AUDIO_FRAMESIZE 20

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvStreamManagement;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;
using namespace chip::app::Clusters::WebRTCTransportProvider;
using namespace chip::app::Clusters::ZoneManagement;
using namespace chip::app::Clusters::AvAnalysis;

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
constexpr int kBallAnimationPattern       = 18;
constexpr guint kTestVideoBackgroundColor = 0xFF808080u;

struct EncoderPrivacyProbeContext
{
    CameraDevice * device      = nullptr;
    bool forceKeyframeOnChange = false;
    std::atomic<uint64_t> lastAppliedGen{ 0 };
    std::atomic<uint64_t> prevSeenGen{ 0 };
};

GQuark GetPrivacyGenQuark()
{
    static const GQuark kQuark = g_quark_from_static_string("chip-privacy-zone-gen");
    return kQuark;
}

void DestroyUint64QData(gpointer data)
{
    delete static_cast<uint64_t *>(data);
}

void LogPrivacyMaskFailureOnce()
{
    static std::atomic<bool> loggedFailure{ false };
    if (!loggedFailure.exchange(true))
    {
        ChipLogError(Camera, "Failed to apply privacy mask to video frame; dropping unmasked frame");
    }
}

GstPadProbeReturn OnEncoderPrivacySinkProbe(GstPad * pad, GstPadProbeInfo * info, gpointer user_data)
{
    auto * holder = static_cast<std::shared_ptr<EncoderPrivacyProbeContext> *>(user_data);
    if (holder == nullptr || *holder == nullptr || (*holder)->device == nullptr)
    {
        return GST_PAD_PROBE_OK;
    }

    GstBuffer * buffer = GST_PAD_PROBE_INFO_BUFFER(info);
    if (buffer == nullptr)
    {
        return GST_PAD_PROBE_OK;
    }

    buffer                        = gst_buffer_make_writable(buffer);
    GST_PAD_PROBE_INFO_DATA(info) = buffer;

    uint64_t appliedGen = 0;
    if (!(*holder)->device->ApplyPrivacyMaskToI420Buffer(pad, buffer, &appliedGen))
    {
        LogPrivacyMaskFailureOnce();
        return GST_PAD_PROBE_DROP;
    }

    const uint64_t prevGen = (*holder)->prevSeenGen.exchange(appliedGen, std::memory_order_acq_rel);
    (*holder)->lastAppliedGen.store(appliedGen, std::memory_order_release);

    if ((*holder)->forceKeyframeOnChange && appliedGen != prevGen)
    {
        GstEvent * forceKeyUnit =
            gst_video_event_new_downstream_force_key_unit(GST_CLOCK_TIME_NONE, GST_CLOCK_TIME_NONE, GST_CLOCK_TIME_NONE, TRUE, 0);
        if (forceKeyUnit != nullptr)
        {
            gst_pad_send_event(pad, forceKeyUnit);
        }
    }

    return GST_PAD_PROBE_OK;
}

GstPadProbeReturn OnEncoderPrivacySrcProbe(GstPad * pad, GstPadProbeInfo * info, gpointer user_data)
{
    auto * holder      = static_cast<std::shared_ptr<EncoderPrivacyProbeContext> *>(user_data);
    GstBuffer * buffer = GST_PAD_PROBE_INFO_BUFFER(info);
    if (holder != nullptr && *holder != nullptr && buffer != nullptr)
    {
        buffer                        = gst_buffer_make_writable(buffer);
        GST_PAD_PROBE_INFO_DATA(info) = buffer;
        const uint64_t gen            = (*holder)->lastAppliedGen.load(std::memory_order_acquire);
        gst_mini_object_set_qdata(GST_MINI_OBJECT_CAST(buffer), GetPrivacyGenQuark(), new uint64_t(gen), DestroyUint64QData);
    }
    return GST_PAD_PROBE_OK;
}

void DestroyEncoderProbeHolder(gpointer data)
{
    delete static_cast<std::shared_ptr<EncoderPrivacyProbeContext> *>(data);
}

void AttachEncoderPrivacyMaskProbes(GstElement * encoder, CameraDevice * device, bool forceKeyframeOnChange)
{
    if (encoder == nullptr || device == nullptr)
    {
        return;
    }

    GstPad * sinkPad = gst_element_get_static_pad(encoder, "sink");
    GstPad * srcPad  = gst_element_get_static_pad(encoder, "src");
    if (sinkPad != nullptr && srcPad != nullptr)
    {
        auto sharedCtx                   = std::make_shared<EncoderPrivacyProbeContext>();
        sharedCtx->device                = device;
        sharedCtx->forceKeyframeOnChange = forceKeyframeOnChange;

        gst_pad_add_probe(sinkPad, GST_PAD_PROBE_TYPE_BUFFER, OnEncoderPrivacySinkProbe,
                          new std::shared_ptr<EncoderPrivacyProbeContext>(sharedCtx), DestroyEncoderProbeHolder);
        gst_pad_add_probe(srcPad, GST_PAD_PROBE_TYPE_BUFFER, OnEncoderPrivacySrcProbe,
                          new std::shared_ptr<EncoderPrivacyProbeContext>(sharedCtx), DestroyEncoderProbeHolder);
    }
    if (sinkPad != nullptr)
    {
        gst_object_unref(sinkPad);
    }
    if (srcPad != nullptr)
    {
        gst_object_unref(srcPad);
    }
}

constexpr int kMaxSnapshotBranchFrameRate = 5;

struct StreamBranchBusContext
{
    std::atomic<bool> faultScheduled{ false };
};

void DestroyStreamBranchBusContext(gpointer data)
{
    delete static_cast<StreamBranchBusContext *>(data);
}

GQuark GetStreamBranchQuark()
{
    static const GQuark kQuark = g_quark_from_static_string("chip-camera-stream-branch");
    return kQuark;
}

void OnCaptureDecodebinPadAdded(GstElement * /* element */, GstPad * newPad, gpointer data)
{
    auto * downstreamElement = static_cast<GstElement *>(data);
    if (downstreamElement == nullptr)
    {
        return;
    }

    GstPad * sinkPad = gst_element_get_static_pad(downstreamElement, "sink");
    if (sinkPad == nullptr)
    {
        return;
    }

    if (!gst_pad_is_linked(sinkPad))
    {
        GstCaps * padCaps = gst_pad_get_current_caps(newPad);
        const GstStructure * str =
            (padCaps != nullptr && gst_caps_get_size(padCaps) > 0) ? gst_caps_get_structure(padCaps, 0) : nullptr;
        const gchar * mediaType    = (str != nullptr) ? gst_structure_get_name(str) : nullptr;
        const bool isRawVideoMedia = (mediaType == nullptr) || (g_str_has_prefix(mediaType, "video/") == TRUE);
        if (padCaps != nullptr)
        {
            gst_caps_unref(padCaps);
        }

        if (isRawVideoMedia && gst_pad_link(newPad, sinkPad) != GST_PAD_LINK_OK)
        {
            ChipLogError(Camera, "Failed to link capture decodebin source pad to videoconvert sink pad");
        }
    }
    gst_object_unref(sinkPad);
}

struct CapturePipelineBusContext
{
    CameraDevice * device = nullptr;
    GstElement * pipeline = nullptr;
    std::atomic<bool> active{ true };
};

void DestroyCaptureBusContextHolder(gpointer data)
{
    delete static_cast<std::shared_ptr<CapturePipelineBusContext> *>(data);
}

GstBusSyncReply OnCapturePipelineBusSyncMessage(GstBus * bus, GstMessage * msg, gpointer user_data)
{
    if (msg == nullptr)
    {
        return GST_BUS_PASS;
    }

    auto * holder = static_cast<std::shared_ptr<CapturePipelineBusContext> *>(user_data);
    if (holder == nullptr || *holder == nullptr || !(*holder)->active.load(std::memory_order_acquire))
    {
        return GST_BUS_DROP;
    }

    const gchar * srcName = (GST_MESSAGE_SRC(msg) != nullptr) ? GST_OBJECT_NAME(GST_MESSAGE_SRC(msg)) : "unknown";

    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_WARNING)
    {
        GError * err       = nullptr;
        gchar * debug_info = nullptr;
        gst_message_parse_warning(msg, &err, &debug_info);
        ChipLogProgress(Camera, "Capture pipeline warning (%s): %s", srcName != nullptr ? srcName : "unknown",
                        err != nullptr ? err->message : "unknown");
        if (debug_info != nullptr)
        {
            ChipLogProgress(Camera, "Capture pipeline warning debug: %s", debug_info);
        }
        g_clear_error(&err);
        g_free(debug_info);
    }
    else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR)
    {
        GError * err       = nullptr;
        gchar * debug_info = nullptr;
        gst_message_parse_error(msg, &err, &debug_info);
        ChipLogError(Camera, "Capture pipeline error (%s): %s", srcName != nullptr ? srcName : "unknown",
                     err != nullptr ? err->message : "unknown");
        if (debug_info != nullptr)
        {
            ChipLogError(Camera, "Capture pipeline error debug: %s", debug_info);
        }
        g_clear_error(&err);
        g_free(debug_info);

        CameraDevice * device = (*holder)->device;
        GstElement * pipeline = (*holder)->pipeline;
        if (device != nullptr && pipeline != nullptr)
        {
            GstElement * faultedBranch         = nullptr;
            StreamBranchBusContext * branchCtx = nullptr;
            bool fromSharedCapture             = false;

            if (GST_MESSAGE_SRC(msg) != nullptr)
            {
                GstObject * current = GST_OBJECT_CAST(gst_object_ref(GST_MESSAGE_SRC(msg)));
                while (current != nullptr)
                {
                    if (GST_IS_ELEMENT(current))
                    {
                        branchCtx =
                            static_cast<StreamBranchBusContext *>(g_object_get_qdata(G_OBJECT(current), GetStreamBranchQuark()));
                        if (branchCtx != nullptr)
                        {
                            faultedBranch = GST_ELEMENT_CAST(current);
                            break;
                        }
                    }

                    if (current == GST_OBJECT_CAST(pipeline))
                    {
                        fromSharedCapture = true;
                        gst_object_unref(current);
                        break;
                    }

                    GstObject * parent = gst_object_get_parent(current);
                    gst_object_unref(current);
                    current = parent;
                }
            }

            if (faultedBranch != nullptr && branchCtx != nullptr)
            {
                if (!branchCtx->faultScheduled.exchange(true, std::memory_order_acq_rel))
                {
                    GstElement * faultedPipeline = static_cast<GstElement *>(gst_object_ref(pipeline));
                    const CHIP_ERROR scheduleErr =
                        DeviceLayer::SystemLayer().ScheduleLambda([device, faultedPipeline, faultedBranch]() {
                            device->HandleBranchRuntimeError(faultedPipeline, faultedBranch);
                            gst_object_unref(faultedBranch);
                            gst_object_unref(faultedPipeline);
                        });
                    if (scheduleErr != CHIP_NO_ERROR)
                    {
                        branchCtx->faultScheduled.store(false, std::memory_order_release);
                        gst_object_unref(faultedBranch);
                        gst_object_unref(faultedPipeline);
                    }
                }
                else
                {
                    gst_object_unref(faultedBranch);
                }
            }
            else if (fromSharedCapture && (*holder)->active.exchange(false, std::memory_order_acq_rel))
            {
                GstElement * faultedPipeline = static_cast<GstElement *>(gst_object_ref(pipeline));
                const CHIP_ERROR scheduleErr = DeviceLayer::SystemLayer().ScheduleLambda([device, faultedPipeline]() {
                    device->HandleCapturePipelineRuntimeError(faultedPipeline);
                    gst_object_unref(faultedPipeline);
                });
                if (scheduleErr != CHIP_NO_ERROR)
                {
                    (*holder)->active.store(true, std::memory_order_release);
                    gst_object_unref(faultedPipeline);
                }
            }
        }
    }

    // Consume messages in the sync handler since no async GstBus watch pops from the shared capture bus.
    return GST_BUS_DROP;
}

struct TeeUnlinkProbeContext
{
    GstPad * sinkPad = nullptr;
    std::mutex mutex;
    std::condition_variable cv;
    bool unlinked = false;

    ~TeeUnlinkProbeContext()
    {
        if (sinkPad != nullptr)
        {
            gst_object_unref(sinkPad);
        }
    }
};

void DestroyTeeUnlinkProbeHolder(gpointer data)
{
    delete static_cast<std::shared_ptr<TeeUnlinkProbeContext> *>(data);
}

GstPadProbeReturn OnTeeSrcPadIdleUnlinkProbe(GstPad * pad, GstPadProbeInfo * info, gpointer user_data)
{
    auto * holder = static_cast<std::shared_ptr<TeeUnlinkProbeContext> *>(user_data);
    if (holder != nullptr && *holder != nullptr)
    {
        std::lock_guard<std::mutex> lock((*holder)->mutex);
        if (!(*holder)->unlinked && (*holder)->sinkPad != nullptr)
        {
            gst_pad_unlink(pad, (*holder)->sinkPad);
            (*holder)->unlinked = true;
        }
        (*holder)->cv.notify_all();
    }
    return GST_PAD_PROBE_REMOVE;
}

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

    const auto * bufferGen =
        static_cast<const uint64_t *>(gst_mini_object_get_qdata(GST_MINI_OBJECT_CAST(buffer), GetPrivacyGenQuark()));
    if (bufferGen == nullptr || *bufferGen < self->GetPrivacyZoneGeneration())
    {
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }

    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_READ))
    {
        GstClockTime rawPts = GST_BUFFER_PTS(buffer);
        if (rawPts == GST_CLOCK_TIME_NONE)
        {
            rawPts = GST_BUFFER_DTS(buffer);
            if (rawPts == GST_CLOCK_TIME_NONE)
            {
                rawPts = 0;
            }
        }
        int64_t ts         = 0;
        int64_t firstPtsMs = 0;
        if (self->ComputeVideoStreamPtsMs(videoStreamID, rawPts, ts, firstPtsMs))
        {
            // Forward raw H.264 encoded frames to media controller with timestamp,
            // serialized with privacy zone generation updates and pre-roll buffer clearing.
            self->DistributeVideoIfCurrentGeneration(reinterpret_cast<const uint8_t *>(map.data), map.size, videoStreamID, ts,
                                                     *bufferGen);
        }
        else
        {
            ChipLogError(Camera,
                         "Dropping video frame with PTS %" G_GUINT64_FORMAT " <= first PTS %" G_GINT64_FORMAT " for stream %u",
                         rawPts, firstPtsMs, videoStreamID);
        }
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
        GstClockTime rawPts = GST_BUFFER_PTS(buffer);
        if (rawPts == GST_CLOCK_TIME_NONE)
        {
            rawPts = GST_BUFFER_DTS(buffer);
            if (rawPts == GST_CLOCK_TIME_NONE)
            {
                rawPts = 0;
            }
        }
        int64_t ts         = 0;
        int64_t firstPtsMs = 0;
        if (self->ComputeAudioStreamPtsMs(audioStreamID, rawPts, ts, firstPtsMs))
        {
            // Forward raw Opus encoded frames to media controller with timestamp
            // The PreRollBuffer will distribute to ALL transports registered for this audioStreamID
            // Each transport will handle its own SFrame encryption (if configured) during RTP packetization
            self->GetMediaController().DistributeAudio(reinterpret_cast<const uint8_t *>(map.data), map.size, audioStreamID, ts);
        }
        else
        {
            ChipLogError(Camera,
                         "Dropping audio frame with PTS %" G_GUINT64_FORMAT " <= first PTS %" G_GINT64_FORMAT " for stream %u",
                         rawPts, firstPtsMs, audioStreamID);
        }
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
    mAVAnalysisManager.SetCameraDevice(this);
}

CameraDevice::~CameraDevice()
{
    StopCameraCapturePipeline();

    if (videoDeviceFd != -1)
    {
        close(videoDeviceFd);
    }
}

void CameraDevice::Init()
{
    CameraError initErr = InitializeCameraDevice();
    if (initErr != CameraError::SUCCESS)
    {
        // Continue initializing logical stream tables so headless cluster tests without physical camera hardware still function.
        ChipLogError(Camera, "Camera hardware initialization failed (err=%d); media capture will fail until a device is available",
                     static_cast<int>(initErr));
    }
    InitializeStreams();
    mWebRTCProviderManager.Init();
    mPushAVTransportManager.Init();
}

void CameraDevice::Shutdown()
{
    // Close WebRTC connections while the SystemLayer is still alive, so that WebRTC callbacks can safely use ScheduleLambda.
    mWebRTCProviderManager.CloseConnection();

    // Stop Video, Audio, and Snapshot Streams so their threads don't access CameraDevice members (like mAudioStreamPtsOffsetMs)
    // after destruction.
    for (auto & stream : mVideoStreams)
    {
        StopVideoStream(stream.videoStreamParams.videoStreamID);
    }
    for (auto & stream : mAudioStreams)
    {
        StopAudioStream(stream.audioStreamParams.audioStreamID);
    }
    for (auto & stream : mSnapshotStreams)
    {
        StopSnapshotStream(stream.snapshotStreamParams.snapshotStreamID);
    }

    StopCameraCapturePipeline();
}

CameraError CameraDevice::InitializeCameraDevice()
{
    static bool gstreamerInitialized = false;

    if (!gstreamerInitialized)
    {
        gst_init(nullptr, nullptr);
        gstreamerInitialized = true;
    }

    if (LinuxDeviceOptions::GetInstance().cameraTestVideosrc)
    {
        ChipLogProgress(Camera, "Using test video source, skipping physical camera initialization");
        return CameraError::SUCCESS;
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

GstElement * CameraDevice::CreateCameraCapturePipeline(const std::string & device, CameraError & error)
{
    GstElement * pipeline     = gst_pipeline_new("camera-capture-pipeline");
    GstElement * sourceCaps   = gst_element_factory_make("capsfilter", "source_caps");
    GstElement * videoconvert = gst_element_factory_make("videoconvert", "capture_videoconvert");
    GstElement * formatCaps   = gst_element_factory_make("capsfilter", "format_caps");
    GstElement * streamTee    = gst_element_factory_make("tee", "stream_tee");
    GstElement * decodebin    = nullptr;
    GstElement * source       = nullptr;

    const bool useTestVideoSrc = LinuxDeviceOptions::GetInstance().cameraTestVideosrc;
    if (useTestVideoSrc)
    {
        source = gst_element_factory_make("videotestsrc", "source");
        if (source != nullptr)
        {
            g_object_set(source, "pattern", kBallAnimationPattern, "background-color", kTestVideoBackgroundColor, "is-live", TRUE,
                         nullptr);
        }
        ChipLogProgress(Camera, "Camera capture pipeline: using test video source");
    }
    else
    {
        decodebin             = gst_element_factory_make("decodebin", "decodebin");
        const auto cameraType = GstreamerPipepline::detectCameraType(device);

        switch (cameraType)
        {
        case GstreamerPipepline::CameraType::kCsi:
            ChipLogDetail(Camera, "Detected CSI camera: %s", device.c_str());
            source = gst_element_factory_make("libcamerasrc", "source");
            break;
        case GstreamerPipepline::CameraType::kUsb:
            ChipLogDetail(Camera, "Detected USB camera: %s", device.c_str());
            source = gst_element_factory_make("v4l2src", "source");
            if (source != nullptr)
            {
                g_object_set(source, "device", device.c_str(), "do-timestamp", TRUE, nullptr);
            }
            break;
        case GstreamerPipepline::CameraType::kFailure:
            if (videoDeviceFd != -1)
            {
                ChipLogProgress(Camera, "Camera capture pipeline: using V4L2 source at %s", device.c_str());
                source = gst_element_factory_make("v4l2src", "source");
                if (source != nullptr)
                {
                    g_object_set(source, "device", device.c_str(), "do-timestamp", TRUE, nullptr);
                }
            }
            else
            {
                ChipLogError(Camera, "Unsupported camera type or device not found: %s", device.c_str());
                GstreamerPipepline::unrefGstElements(pipeline, sourceCaps, decodebin, videoconvert, formatCaps, streamTee);
                error = CameraError::ERROR_INIT_FAILED;
                return nullptr;
            }
            break;
        }
    }

    std::vector<std::pair<GstElement *, const char *>> elements = {
        { pipeline, "pipeline" },         //
        { source, "source" },             //
        { sourceCaps, "source_caps" },    //
        { videoconvert, "videoconvert" }, //
        { formatCaps, "format_caps" },    //
        { streamTee, "stream_tee" }       //
    };
    if (!useTestVideoSrc)
    {
        elements.emplace_back(decodebin, "decodebin");
    }
    if (GstreamerPipepline::isGstElementsNull(elements))
    {
        ChipLogError(Camera, "Not all camera capture pipeline elements could be created.");
        GstreamerPipepline::unrefGstElements(pipeline, source, sourceCaps, decodebin, videoconvert, formatCaps, streamTee);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    g_object_set(streamTee, "allow-not-linked", TRUE, nullptr);

    GstCaps * srcCaps = nullptr;
    if (useTestVideoSrc)
    {
        const uint16_t captureFps =
            std::clamp<uint16_t>(LinuxDeviceOptions::GetInstance().cameraFramerate.ValueOr(k30fpsVideoFrameRate),
                                 kMinVideoFrameRate, kMaxVideoFrameRate);
        srcCaps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420", "width", G_TYPE_INT,
                                      static_cast<int>(k720pResolutionWidth), "height", G_TYPE_INT,
                                      static_cast<int>(k720pResolutionHeight), "framerate", GST_TYPE_FRACTION,
                                      static_cast<int>(captureFps), 1, nullptr);
    }
    else
    {
        // Prefer streaming-rate raw capture first, followed by streaming-rate MJPEG (for MJPEG-only or USB 2.0 HD cameras),
        // with unconstrained-fps raw/MJPEG fallbacks bounded to kMaxResolution so cameras never pick multi-megapixel modes.
        srcCaps = gst_caps_new_empty();
        gst_caps_append_structure(
            srcCaps,
            gst_structure_new("video/x-raw", "width", GST_TYPE_INT_RANGE, 1, static_cast<int>(kMaxResolutionWidth), "height",
                              GST_TYPE_INT_RANGE, 1, static_cast<int>(kMaxResolutionHeight), "framerate", GST_TYPE_FRACTION_RANGE,
                              static_cast<int>(kMinVideoFrameRate), 1, static_cast<int>(kMaxVideoFrameRate), 1, nullptr));
        gst_caps_append_structure(
            srcCaps,
            gst_structure_new("image/jpeg", "width", GST_TYPE_INT_RANGE, 1, static_cast<int>(kMaxResolutionWidth), "height",
                              GST_TYPE_INT_RANGE, 1, static_cast<int>(kMaxResolutionHeight), "framerate", GST_TYPE_FRACTION_RANGE,
                              static_cast<int>(kMinVideoFrameRate), 1, static_cast<int>(kMaxVideoFrameRate), 1, nullptr));
        gst_caps_append_structure(srcCaps,
                                  gst_structure_new("video/x-raw", "width", GST_TYPE_INT_RANGE, 1,
                                                    static_cast<int>(kMaxResolutionWidth), "height", GST_TYPE_INT_RANGE, 1,
                                                    static_cast<int>(kMaxResolutionHeight), nullptr));
        gst_caps_append_structure(srcCaps,
                                  gst_structure_new("image/jpeg", "width", GST_TYPE_INT_RANGE, 1,
                                                    static_cast<int>(kMaxResolutionWidth), "height", GST_TYPE_INT_RANGE, 1,
                                                    static_cast<int>(kMaxResolutionHeight), nullptr));
    }
    g_object_set(sourceCaps, "caps", srcCaps, nullptr);
    gst_caps_unref(srcCaps);

    GstCaps * i420Caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420", nullptr);
    g_object_set(formatCaps, "caps", i420Caps, nullptr);
    gst_caps_unref(i420Caps);

    if (useTestVideoSrc)
    {
        gst_bin_add_many(GST_BIN(pipeline), source, sourceCaps, videoconvert, formatCaps, streamTee, nullptr);
        if (!gst_element_link_many(source, sourceCaps, videoconvert, formatCaps, streamTee, nullptr))
        {
            ChipLogError(Camera, "CreateCameraCapturePipeline: link failed");
            gst_object_unref(pipeline);
            error = CameraError::ERROR_INIT_FAILED;
            return nullptr;
        }
    }
    else
    {
        gst_bin_add_many(GST_BIN(pipeline), source, sourceCaps, decodebin, videoconvert, formatCaps, streamTee, nullptr);
        if (!gst_element_link_many(source, sourceCaps, decodebin, nullptr) ||
            !gst_element_link_many(videoconvert, formatCaps, streamTee, nullptr))
        {
            ChipLogError(Camera, "CreateCameraCapturePipeline: hardware capture link failed");
            gst_object_unref(pipeline);
            error = CameraError::ERROR_INIT_FAILED;
            return nullptr;
        }
        g_signal_connect(decodebin, "pad-added", G_CALLBACK(OnCaptureDecodebinPadAdded), videoconvert);
    }

    GstBus * bus = gst_element_get_bus(pipeline);
    if (bus != nullptr)
    {
        auto busCtx      = std::make_shared<CapturePipelineBusContext>();
        busCtx->device   = this;
        busCtx->pipeline = pipeline;
        gst_bus_set_sync_handler(bus, OnCapturePipelineBusSyncMessage, new std::shared_ptr<CapturePipelineBusContext>(busCtx),
                                 DestroyCaptureBusContextHolder);
        gst_object_unref(bus);
    }

    return pipeline;
}

GstElement * CameraDevice::CreateSnapshotBranchBin(uint16_t streamID, int width, int height, int quality, int framerate,
                                                   CameraError & error)
{
    std::string binName     = "snapshot-branch-" + std::to_string(streamID);
    GstElement * bin        = gst_bin_new(binName.c_str());
    GstElement * queue      = gst_element_factory_make("queue", "queue");
    GstElement * videoscale = gst_element_factory_make("videoscale", "videoscale");
    GstElement * videorate  = gst_element_factory_make("videorate", "videorate");
    GstElement * capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
    GstElement * jpegenc    = gst_element_factory_make("jpegenc", "jpegenc");
    GstElement * appsink    = gst_element_factory_make("appsink", "sink");

    const std::vector<std::pair<GstElement *, const char *>> elements = {
        { bin, "bin" },               //
        { queue, "queue" },           //
        { videoscale, "videoscale" }, //
        { videorate, "videorate" },   //
        { capsfilter, "capsfilter" }, //
        { jpegenc, "jpegenc" },       //
        { appsink, "appsink" }        //
    };
    if (GstreamerPipepline::isGstElementsNull(elements))
    {
        ChipLogError(Camera, "Not all snapshot branch elements could be created.");
        GstreamerPipepline::unrefGstElements(bin, queue, videoscale, videorate, capsfilter, jpegenc, appsink);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    g_object_set(queue, "max-size-buffers", 1, "leaky", 2 /* downstream */, nullptr);
    g_object_set(videorate, "skip-to-first", TRUE, nullptr);

    // SnapshotStreamAllocate's maxFrameRate is an upper bound; cap continuous JPEG encoding rate to save idle CPU.
    const int effectiveFrameRate = std::clamp(framerate, 1, kMaxSnapshotBranchFrameRate);
    GstCaps * caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420", "width", G_TYPE_INT, width, "height",
                                         G_TYPE_INT, height, "framerate", GST_TYPE_FRACTION, effectiveFrameRate, 1, nullptr);
    g_object_set(capsfilter, "caps", caps, nullptr);
    gst_caps_unref(caps);

    g_object_set(jpegenc, "quality", quality, nullptr);
    g_object_set(appsink, "emit-signals", FALSE, "sync", FALSE, "async", FALSE, "max-buffers", 1, "drop", TRUE, nullptr);

    gst_bin_add_many(GST_BIN(bin), queue, videoscale, videorate, capsfilter, jpegenc, appsink, nullptr);
    if (!gst_element_link_many(queue, videoscale, videorate, capsfilter, jpegenc, appsink, nullptr))
    {
        ChipLogError(Camera, "CreateSnapshotBranchBin: link failed");
        gst_object_ref_sink(bin);
        gst_object_unref(bin);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    AttachEncoderPrivacyMaskProbes(jpegenc, this, /* forceKeyframeOnChange = */ false);

    GstPad * queueSinkPad = gst_element_get_static_pad(queue, "sink");
    if (queueSinkPad == nullptr)
    {
        ChipLogError(Camera, "CreateSnapshotBranchBin: failed to get queue sink pad");
        gst_object_ref_sink(bin);
        gst_object_unref(bin);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }
    GstPad * ghostSinkPad = gst_ghost_pad_new("sink", queueSinkPad);
    gst_object_unref(queueSinkPad);
    gst_pad_set_active(ghostSinkPad, TRUE);
    gst_element_add_pad(bin, ghostSinkPad);

    return bin;
}

GstElement * CameraDevice::CreateVideoBranchBin(uint16_t streamID, int width, int height, int framerate, CameraError & error)
{
    std::string binName     = "video-branch-" + std::to_string(streamID);
    GstElement * bin        = gst_bin_new(binName.c_str());
    GstElement * queue      = gst_element_factory_make("queue", "queue");
    GstElement * videoscale = gst_element_factory_make("videoscale", "videoscale");
    GstElement * videorate  = gst_element_factory_make("videorate", "videorate");
    GstElement * capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
    GstElement * x264enc    = gst_element_factory_make("x264enc", "encoder");
    GstElement * appsink    = gst_element_factory_make("appsink", "appsink");

    const std::vector<std::pair<GstElement *, const char *>> elements = {
        { bin, "bin" },               //
        { queue, "queue" },           //
        { videoscale, "videoscale" }, //
        { videorate, "videorate" },   //
        { capsfilter, "capsfilter" }, //
        { x264enc, "encoder" },       //
        { appsink, "appsink" }        //
    };
    if (GstreamerPipepline::isGstElementsNull(elements))
    {
        ChipLogError(Camera, "Not all video branch elements could be created.");
        GstreamerPipepline::unrefGstElements(bin, queue, videoscale, videorate, capsfilter, x264enc, appsink);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    g_object_set(queue, "max-size-buffers", 1, "leaky", 2 /* downstream */, nullptr);
    g_object_set(videorate, "skip-to-first", TRUE, nullptr);

    GstCaps * caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420", "width", G_TYPE_INT, width, "height",
                                         G_TYPE_INT, height, "framerate", GST_TYPE_FRACTION, framerate, 1, nullptr);
    g_object_set(capsfilter, "caps", caps, nullptr);
    gst_caps_unref(caps);

    // Configure encoder for low-latency (0x04 = zerolatency) and force IDR at start
    g_object_set(x264enc, "tune", 0x04, "speed-preset", 1, "key-int-max", framerate * 1, "insert-vui", TRUE, nullptr);

    // Configure appsink for receiving H.264 buffers data
    g_object_set(appsink, "emit-signals", TRUE, "sync", FALSE, "async", FALSE, nullptr);

    VideoAppSinkContext * context = new VideoAppSinkContext{ this, streamID };
    GstAppSinkCallbacks callbacks = { nullptr, nullptr, OnNewVideoSampleFromAppSink };
    gst_app_sink_set_callbacks(GST_APP_SINK(appsink), &callbacks, context, DestroyVideoAppSinkContext);

    gst_bin_add_many(GST_BIN(bin), queue, videoscale, videorate, capsfilter, x264enc, appsink, nullptr);
    if (!gst_element_link_many(queue, videoscale, videorate, capsfilter, x264enc, appsink, nullptr))
    {
        ChipLogError(Camera, "CreateVideoBranchBin: link failed");
        gst_object_ref_sink(bin);
        gst_object_unref(bin);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }

    AttachEncoderPrivacyMaskProbes(x264enc, this, /* forceKeyframeOnChange = */ true);

    GstPad * queueSinkPad = gst_element_get_static_pad(queue, "sink");
    if (queueSinkPad == nullptr)
    {
        ChipLogError(Camera, "CreateVideoBranchBin: failed to get queue sink pad");
        gst_object_ref_sink(bin);
        gst_object_unref(bin);
        error = CameraError::ERROR_INIT_FAILED;
        return nullptr;
    }
    GstPad * ghostSinkPad = gst_ghost_pad_new("sink", queueSinkPad);
    gst_object_unref(queueSinkPad);
    gst_pad_set_active(ghostSinkPad, TRUE);
    gst_element_add_pad(bin, ghostSinkPad);

    return bin;
}

bool CameraDevice::HasActiveStreamBranches() const
{
    for (const auto & stream : mVideoStreams)
    {
        if (stream.videoContext != nullptr)
        {
            return true;
        }
    }
    for (const auto & stream : mSnapshotStreams)
    {
        if (stream.snapshotContext != nullptr)
        {
            return true;
        }
    }
    return false;
}

CameraError CameraDevice::StopCameraCapturePipeline(CameraError stopFailedError)
{
    std::lock_guard<std::recursive_mutex> lock(mPipelineMutex);
    if (mCameraTee != nullptr)
    {
        gst_object_unref(mCameraTee);
        mCameraTee = nullptr;
    }
    if (mCameraPipeline != nullptr)
    {
        GstBus * bus = gst_element_get_bus(mCameraPipeline);
        if (bus != nullptr)
        {
            gst_bus_set_sync_handler(bus, nullptr, nullptr, nullptr);
            gst_object_unref(bus);
        }

        GstStateChangeReturn result = gst_element_set_state(mCameraPipeline, GST_STATE_NULL);
        gst_object_unref(mCameraPipeline);
        mCameraPipeline = nullptr;
        if (result == GST_STATE_CHANGE_FAILURE)
        {
            return stopFailedError;
        }
    }
    return CameraError::SUCCESS;
}

void CameraDevice::HandleCapturePipelineRuntimeError(GstElement * faultedPipeline)
{
    bool stoppedAnyVideoStream = false;
    {
        std::lock_guard<std::recursive_mutex> lock(mPipelineMutex);
        if (faultedPipeline == nullptr || mCameraPipeline != faultedPipeline)
        {
            return;
        }

        ChipLogError(Camera, "Stopping active streams and shared camera capture pipeline after runtime error");
        for (auto & stream : mVideoStreams)
        {
            if (stream.videoContext != nullptr)
            {
                StopVideoStream(stream.videoStreamParams.videoStreamID);
                stoppedAnyVideoStream = true;
            }
        }
        for (auto & stream : mSnapshotStreams)
        {
            if (stream.snapshotContext != nullptr)
            {
                StopSnapshotStream(stream.snapshotStreamParams.snapshotStreamID);
            }
        }
        StopCameraCapturePipeline();
    }

    if (stoppedAnyVideoStream)
    {
        mCameraAVStreamManager.SyncCurrentFrameRateFromHAL();
    }
}

void CameraDevice::HandleBranchRuntimeError(GstElement * faultedPipeline, GstElement * faultedBranch)
{
    bool stoppedAnyVideoStream = false;
    {
        std::lock_guard<std::recursive_mutex> lock(mPipelineMutex);
        if (faultedPipeline == nullptr || faultedBranch == nullptr || mCameraPipeline != faultedPipeline)
        {
            return;
        }

        for (auto & stream : mVideoStreams)
        {
            if (stream.videoContext == faultedBranch)
            {
                ChipLogError(Camera, "Stopping faulted video stream branch (id=%u) after runtime error",
                             stream.videoStreamParams.videoStreamID);
                StopVideoStream(stream.videoStreamParams.videoStreamID);
                stoppedAnyVideoStream = true;
                break;
            }
        }

        if (!stoppedAnyVideoStream)
        {
            for (auto & stream : mSnapshotStreams)
            {
                if (stream.snapshotContext == faultedBranch)
                {
                    ChipLogError(Camera, "Stopping faulted snapshot stream branch (id=%u) after runtime error",
                                 stream.snapshotStreamParams.snapshotStreamID);
                    StopSnapshotStream(stream.snapshotStreamParams.snapshotStreamID);
                    break;
                }
            }
        }
    }

    if (stoppedAnyVideoStream)
    {
        mCameraAVStreamManager.SyncCurrentFrameRateFromHAL();
    }
}

CameraError CameraDevice::AttachStreamBranch(GstElement * branchBin, CameraError startFailedError)
{
    std::lock_guard<std::recursive_mutex> lock(mPipelineMutex);
    if (branchBin == nullptr)
    {
        return startFailedError;
    }

    const bool createdPipeline = (mCameraPipeline == nullptr);
    if (createdPipeline)
    {
        CameraError error = CameraError::SUCCESS;
        mCameraPipeline   = CreateCameraCapturePipeline(mVideoDevicePath, error);
        if (mCameraPipeline == nullptr)
        {
            ChipLogError(Camera, "Failed to create shared camera capture pipeline.");
            gst_object_ref_sink(branchBin);
            gst_object_unref(branchBin);
            return error;
        }
        mCameraTee = gst_bin_get_by_name(GST_BIN(mCameraPipeline), "stream_tee");
        if (mCameraTee == nullptr)
        {
            ChipLogError(Camera, "Failed to find stream_tee in camera capture pipeline.");
            gst_object_ref_sink(branchBin);
            gst_object_unref(branchBin);
            StopCameraCapturePipeline();
            return CameraError::ERROR_INIT_FAILED;
        }
    }

    g_object_set_qdata_full(G_OBJECT(branchBin), GetStreamBranchQuark(), new StreamBranchBusContext(),
                            DestroyStreamBranchBusContext);

    if (!gst_bin_add(GST_BIN(mCameraPipeline), branchBin))
    {
        ChipLogError(Camera, "Failed to add stream branch bin to camera capture pipeline.");
        gst_object_ref_sink(branchBin);
        gst_object_unref(branchBin);
        if (createdPipeline)
        {
            StopCameraCapturePipeline();
        }
        return startFailedError;
    }

    if (!createdPipeline)
    {
        gst_element_sync_state_with_parent(branchBin);
    }

    GstPad * teeSrcPad     = gst_element_request_pad_simple(mCameraTee, "src_%u");
    GstPad * branchSinkPad = gst_element_get_static_pad(branchBin, "sink");
    if (teeSrcPad == nullptr || branchSinkPad == nullptr || gst_pad_link(teeSrcPad, branchSinkPad) != GST_PAD_LINK_OK)
    {
        ChipLogError(Camera, "Failed to link tee source pad to stream branch sink pad.");
        if (branchSinkPad != nullptr)
        {
            gst_object_unref(branchSinkPad);
        }
        if (teeSrcPad != nullptr)
        {
            gst_element_release_request_pad(mCameraTee, teeSrcPad);
            gst_object_unref(teeSrcPad);
        }
        if (createdPipeline)
        {
            StopCameraCapturePipeline();
        }
        else
        {
            gst_element_set_state(branchBin, GST_STATE_NULL);
            gst_bin_remove(GST_BIN(mCameraPipeline), branchBin);
        }
        return startFailedError;
    }
    gst_object_unref(branchSinkPad);
    gst_object_unref(teeSrcPad);

    GstElement * stateTarget = createdPipeline ? mCameraPipeline : branchBin;
    if (createdPipeline)
    {
        GstStateChangeReturn result = gst_element_set_state(mCameraPipeline, GST_STATE_PLAYING);
        if (result == GST_STATE_CHANGE_FAILURE)
        {
            ChipLogError(Camera, "Failed to set shared camera capture pipeline to PLAYING.");
            StopCameraCapturePipeline();
            return startFailedError;
        }
    }

    GstState state = GST_STATE_NULL;
    GstStateChangeReturn state_result =
        gst_element_get_state(stateTarget, &state, nullptr, VIDEO_PIPELINE_PLAY_TIMEOUT * GST_SECOND);
    if (state_result == GST_STATE_CHANGE_FAILURE || state != GST_STATE_PLAYING)
    {
        ChipLogError(Camera, "Stream branch did not reach PLAYING state. Result: %d, State: %d", state_result, state);

        if (createdPipeline)
        {
            StopCameraCapturePipeline();
        }
        else
        {
            DetachStreamBranch(branchBin, startFailedError);
        }
        return startFailedError;
    }

    return CameraError::SUCCESS;
}

CameraError CameraDevice::DetachStreamBranch(GstElement * branchBin, CameraError stopFailedError)
{
    std::lock_guard<std::recursive_mutex> lock(mPipelineMutex);
    if (branchBin == nullptr)
    {
        return CameraError::SUCCESS;
    }

    if (mCameraPipeline == nullptr)
    {
        return StopCameraCapturePipeline(stopFailedError);
    }

    gst_object_ref(branchBin);

    GstPad * branchSinkPad = gst_element_get_static_pad(branchBin, "sink");
    if (branchSinkPad != nullptr)
    {
        GstPad * teeSrcPad = gst_pad_get_peer(branchSinkPad);
        if (teeSrcPad != nullptr)
        {
            auto probeCtx     = std::make_shared<TeeUnlinkProbeContext>();
            probeCtx->sinkPad = static_cast<GstPad *>(gst_object_ref(branchSinkPad));
            const gulong probeId =
                gst_pad_add_probe(teeSrcPad, GST_PAD_PROBE_TYPE_IDLE, OnTeeSrcPadIdleUnlinkProbe,
                                  new std::shared_ptr<TeeUnlinkProbeContext>(probeCtx), DestroyTeeUnlinkProbeHolder);

            bool unlinkedInProbe = false;
            {
                std::unique_lock<std::mutex> probeLock(probeCtx->mutex);
                unlinkedInProbe =
                    probeCtx->cv.wait_for(probeLock, std::chrono::milliseconds(500), [&probeCtx]() { return probeCtx->unlinked; });
                if (!unlinkedInProbe)
                {
                    probeCtx->unlinked = true;
                }
            }
            if (!unlinkedInProbe)
            {
                if (probeId != 0)
                {
                    gst_pad_remove_probe(teeSrcPad, probeId);
                }
                gst_pad_unlink(teeSrcPad, branchSinkPad);
            }

            if (mCameraTee != nullptr)
            {
                gst_element_release_request_pad(mCameraTee, teeSrcPad);
            }
            gst_object_unref(teeSrcPad);
        }
        gst_object_unref(branchSinkPad);
    }

    GstStateChangeReturn result = gst_element_set_state(branchBin, GST_STATE_NULL);
    gst_bin_remove(GST_BIN(mCameraPipeline), branchBin);
    gst_object_unref(branchBin);

    if (!HasActiveStreamBranches())
    {
        CameraError pipelineError = StopCameraCapturePipeline(stopFailedError);
        if (result == GST_STATE_CHANGE_FAILURE || pipelineError != CameraError::SUCCESS)
        {
            return stopFailedError;
        }
        return CameraError::SUCCESS;
    }

    if (result == GST_STATE_CHANGE_FAILURE)
    {
        return stopFailedError;
    }

    return CameraError::SUCCESS;
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
        if (!stream.isAllocated)
        {
            continue;
        }

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
    std::unique_lock<std::recursive_mutex> lock(mPipelineMutex);
    VideoResolutionStruct matchedRes;
    ImageCodecEnum matchedCodec;
    uint16_t streamId               = 0;
    SnapshotStream * snapshotStream = nullptr;

    if (streamID.IsNull())
    {
        if (!MatchClosestSnapshotParams(resolution, matchedRes, matchedCodec))
        {
            ChipLogError(Camera, "No matching snapshot stream found for requested resolution %ux%u", resolution.width,
                         resolution.height);
            return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
        }
        // Find the stream that matched
        for (auto & s : mSnapshotStreams)
        {
            if (s.isAllocated && s.snapshotStreamParams.minResolution.width == matchedRes.width &&
                s.snapshotStreamParams.minResolution.height == matchedRes.height &&
                s.snapshotStreamParams.imageCodec == matchedCodec)
            {
                snapshotStream = &s;
                streamId       = s.snapshotStreamParams.snapshotStreamID;
                break;
            }
        }
    }
    else
    {
        streamId = streamID.Value();
        auto it  = std::find_if(mSnapshotStreams.begin(), mSnapshotStreams.end(), [streamId](const SnapshotStream & s) {
            return s.isAllocated && s.snapshotStreamParams.snapshotStreamID == streamId;
        });
        if (it == mSnapshotStreams.end())
        {
            ChipLogError(Camera, "Snapshot stream not found for stream ID %u", streamId);
            return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
        }
        snapshotStream = &(*it);
        matchedRes     = it->snapshotStreamParams.minResolution;
        matchedCodec   = it->snapshotStreamParams.imageCodec;
    }

    if (snapshotStream == nullptr)
    {
        ChipLogError(Camera, "Failed to determine snapshot stream");
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    GstElement * snapshotPipeline = reinterpret_cast<GstElement *>(snapshotStream->snapshotContext);
    bool startedOnDemand          = false;

    if (snapshotPipeline == nullptr)
    {
        // Stream is not running, start it temporarily
        ChipLogProgress(Camera, "Snapshot stream not running, starting on demand");
        if (StartSnapshotStream(streamId) != CameraError::SUCCESS)
        {
            ChipLogError(Camera, "Failed to start snapshot stream on demand");
            return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
        }
        snapshotPipeline = reinterpret_cast<GstElement *>(snapshotStream->snapshotContext);
        startedOnDemand  = true;
    }

    if (snapshotPipeline == nullptr)
    {
        ChipLogError(Camera, "Snapshot pipeline is still null after trying to start");
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    auto stopOnDemandStream = [this, streamId, startedOnDemand, &lock]() {
        if (startedOnDemand)
        {
            if (!lock.owns_lock())
            {
                lock.lock();
            }
            ChipLogProgress(Camera, "Stopping snapshot stream that was started on demand");
            if (StopSnapshotStream(streamId) != CameraError::SUCCESS)
            {
                ChipLogError(Camera, "Failed to stop snapshot stream %u that was started on demand", streamId);
            }
        }
    };

    // Get the appsink (returns a new owned reference, safe to hold while mPipelineMutex is unlocked)
    GstElement * appsink = gst_bin_get_by_name(GST_BIN(snapshotPipeline), "sink");
    if (!appsink)
    {
        ChipLogError(Camera, "Failed to get appsink from snapshot pipeline");
        stopOnDemandStream();
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    lock.unlock();

    // Pull sample from appsink whose privacy mask generation is at least requiredGen so pre-update frames are never returned.
    ChipLogProgress(Camera, "Pulling sample from appsink...");
    const uint64_t requiredGen = GetPrivacyZoneGeneration();
    const auto pullDeadline    = std::chrono::steady_clock::now() + std::chrono::seconds(4);
    GstSample * sample         = nullptr;

    while (std::chrono::steady_clock::now() < pullDeadline)
    {
        const auto remainingNs =
            std::chrono::duration_cast<std::chrono::nanoseconds>(pullDeadline - std::chrono::steady_clock::now());
        if (remainingNs.count() <= 0)
        {
            break;
        }

        GstSample * candidate = gst_app_sink_try_pull_sample(GST_APP_SINK(appsink), static_cast<GstClockTime>(remainingNs.count()));
        if (candidate == nullptr)
        {
            break;
        }

        GstBuffer * candidateBuf = gst_sample_get_buffer(candidate);
        const auto * frameGenPtr = (candidateBuf != nullptr)
            ? static_cast<const uint64_t *>(gst_mini_object_get_qdata(GST_MINI_OBJECT_CAST(candidateBuf), GetPrivacyGenQuark()))
            : nullptr;
        if (frameGenPtr != nullptr && *frameGenPtr >= requiredGen)
        {
            sample = candidate;
            break;
        }

        gst_sample_unref(candidate);
    }
    gst_object_unref(appsink);

    if (!sample)
    {
        ChipLogError(Camera, "Failed to pull sample from appsink (timeout or error)");
        stopOnDemandStream();
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    GstBuffer * buffer = gst_sample_get_buffer(sample);
    if (!buffer)
    {
        ChipLogError(Camera, "Failed to get buffer from sample");
        gst_sample_unref(sample);
        stopOnDemandStream();
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    GstMapInfo map;

    if (gst_buffer_map(buffer, &map, GST_MAP_READ))
    {
        outImageSnapshot.data.resize(map.size);
        memcpy(outImageSnapshot.data.data(), map.data, map.size);
        gst_buffer_unmap(buffer, &map);
    }
    else
    {
        ChipLogError(Camera, "Failed to map buffer");
        gst_sample_unref(sample);
        stopOnDemandStream();
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    gst_sample_unref(sample);

    stopOnDemandStream();

    outImageSnapshot.imageRes   = matchedRes;
    outImageSnapshot.imageCodec = matchedCodec;

    return CameraError::SUCCESS;
}

void CameraDevice::UpdateCurrentVideoFrameRate()
{
    uint16_t maxFrameRate = 0;
    for (const auto & stream : mVideoStreams)
    {
        if (stream.videoContext != nullptr)
        {
            auto rateIt = mActiveVideoStreamFrameRates.find(stream.videoStreamParams.videoStreamID);
            if (rateIt != mActiveVideoStreamFrameRates.end())
            {
                maxFrameRate = std::max(maxFrameRate, rateIt->second);
            }
        }
    }
    mCurrentVideoFrameRate = maxFrameRate;
}

bool CameraDevice::ComputeVideoStreamPtsMs(uint16_t videoStreamID, GstClockTime rawPts, int64_t & outTimestampMs,
                                           int64_t & outFirstPtsMs)
{
    std::lock_guard<std::mutex> lock(mPtsOffsetMutex);
    auto firstPtsIt = mVideoStreamPtsOffsetMs.find(videoStreamID);
    if (firstPtsIt == mVideoStreamPtsOffsetMs.end())
    {
        const auto now      = std::chrono::steady_clock::now().time_since_epoch();
        const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
        const int64_t rawMs = static_cast<int64_t>(rawPts / 1000000);
        firstPtsIt          = mVideoStreamPtsOffsetMs.emplace(videoStreamID, nowMs - rawMs).first;
    }
    outFirstPtsMs  = firstPtsIt->second;
    outTimestampMs = outFirstPtsMs + static_cast<int64_t>(rawPts / 1000000);
    return outTimestampMs >= outFirstPtsMs;
}

bool CameraDevice::ComputeAudioStreamPtsMs(uint16_t audioStreamID, GstClockTime rawPts, int64_t & outTimestampMs,
                                           int64_t & outFirstPtsMs)
{
    std::lock_guard<std::mutex> lock(mPtsOffsetMutex);
    auto firstPtsIt = mAudioStreamPtsOffsetMs.find(audioStreamID);
    if (firstPtsIt == mAudioStreamPtsOffsetMs.end())
    {
        const auto now      = std::chrono::steady_clock::now().time_since_epoch();
        const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
        const int64_t rawMs = static_cast<int64_t>(rawPts / 1000000);
        firstPtsIt          = mAudioStreamPtsOffsetMs.emplace(audioStreamID, nowMs - rawMs).first;
    }
    outFirstPtsMs  = firstPtsIt->second;
    outTimestampMs = outFirstPtsMs + static_cast<int64_t>(rawPts / 1000000);
    return outTimestampMs >= outFirstPtsMs;
}

CameraError CameraDevice::StartVideoStream(const VideoStreamStruct & allocatedStream)
{
    std::lock_guard<std::recursive_mutex> lock(mPipelineMutex);
    uint16_t streamID = allocatedStream.videoStreamID;
    auto it           = std::find_if(mVideoStreams.begin(), mVideoStreams.end(),
                                     [streamID](const VideoStream & s) { return s.videoStreamParams.videoStreamID == streamID; });

    if (it == mVideoStreams.end())
    {
        return CameraError::ERROR_VIDEO_STREAM_START_FAILED;
    }

    if (it->videoContext != nullptr)
    {
        StopVideoStream(streamID);
    }

    const uint16_t framerate = std::clamp(LinuxDeviceOptions::GetInstance().cameraFramerate.ValueOr(k30fpsVideoFrameRate),
                                          allocatedStream.minFrameRate, allocatedStream.maxFrameRate);

    CameraError error = CameraError::SUCCESS;
    GstElement * videoBranchBin =
        CreateVideoBranchBin(streamID, allocatedStream.minResolution.width, allocatedStream.minResolution.height, framerate, error);
    if (videoBranchBin == nullptr)
    {
        ChipLogError(Camera, "Failed to create video branch bin.");
        it->videoContext = nullptr;
        UpdateCurrentVideoFrameRate();
        return error;
    }

    ChipLogProgress(Camera, "Starting video stream (id=%u): %u×%u @ %ufps", streamID, allocatedStream.minResolution.width,
                    allocatedStream.minResolution.height, framerate);

    error = AttachStreamBranch(videoBranchBin, CameraError::ERROR_VIDEO_STREAM_START_FAILED);
    if (error != CameraError::SUCCESS)
    {
        ChipLogError(Camera, "Failed to attach and start video stream branch (id=%u).", streamID);
        it->videoContext = nullptr;
        UpdateCurrentVideoFrameRate();
        return error;
    }

    it->videoContext                       = videoBranchBin;
    mActiveVideoStreamFrameRates[streamID] = framerate;
    UpdateCurrentVideoFrameRate();

    ChipLogProgress(Camera, "Video is PLAYING …");

    return CameraError::SUCCESS;
}

// Stop video stream
CameraError CameraDevice::StopVideoStream(uint16_t streamID)
{
    std::lock_guard<std::recursive_mutex> lock(mPipelineMutex);
    auto it = std::find_if(mVideoStreams.begin(), mVideoStreams.end(),
                           [streamID](const VideoStream & s) { return s.videoStreamParams.videoStreamID == streamID; });

    if (it == mVideoStreams.end())
    {
        return CameraError::ERROR_VIDEO_STREAM_STOP_FAILED;
    }

    CameraError error           = CameraError::SUCCESS;
    GstElement * videoBranchBin = reinterpret_cast<GstElement *>(it->videoContext);
    if (videoBranchBin != nullptr)
    {
        it->videoContext = nullptr;
        error            = DetachStreamBranch(videoBranchBin, CameraError::ERROR_VIDEO_STREAM_STOP_FAILED);
    }
    mActiveVideoStreamFrameRates.erase(streamID);
    UpdateCurrentVideoFrameRate();
    {
        std::lock_guard<std::mutex> ptsLock(mPtsOffsetMutex);
        mVideoStreamPtsOffsetMs.erase(streamID);
    }

    return error;
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

    ChipLogProgress(Camera, "Audio Pipeline reached playing state");

    // Wait for the pipeline to reach the PLAYING state
    GstState state;
    gst_element_get_state(audioPipeline, &state, nullptr, 5 * GST_SECOND);
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

        // Always clean up, regardless of state change result
        gst_object_unref(audioPipeline);
        it->audioContext = nullptr;

        if (result == GST_STATE_CHANGE_FAILURE)
        {
            return CameraError::ERROR_AUDIO_STREAM_STOP_FAILED;
        }
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

    {
        std::lock_guard<std::mutex> ptsLock(mPtsOffsetMutex);
        mAudioStreamPtsOffsetMs.erase(streamID);
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
    std::lock_guard<std::recursive_mutex> lock(mPipelineMutex);
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
    std::lock_guard<std::recursive_mutex> lock(mPipelineMutex);
    auto it = std::find_if(mSnapshotStreams.begin(), mSnapshotStreams.end(),
                           [streamID](const SnapshotStream & s) { return s.snapshotStreamParams.snapshotStreamID == streamID; });
    if (it == mSnapshotStreams.end())
    {
        ChipLogError(Camera, "Snapshot streamID : %u not found", streamID);
        return CameraError::ERROR_SNAPSHOT_STREAM_START_FAILED;
    }

    if (it->snapshotContext != nullptr)
    {
        StopSnapshotStream(streamID);
    }

    CameraError error              = CameraError::SUCCESS;
    GstElement * snapshotBranchBin = CreateSnapshotBranchBin(
        streamID, it->snapshotStreamParams.minResolution.width, it->snapshotStreamParams.minResolution.height,
        it->snapshotStreamParams.quality, it->snapshotStreamParams.frameRate, error);
    if (snapshotBranchBin == nullptr)
    {
        ChipLogError(Camera, "Failed to create snapshot branch bin.");
        it->snapshotContext = nullptr;
        return error;
    }

    error = AttachStreamBranch(snapshotBranchBin, CameraError::ERROR_SNAPSHOT_STREAM_START_FAILED);
    if (error != CameraError::SUCCESS)
    {
        ChipLogError(Camera, "Failed to attach and start snapshot stream branch (id=%u).", streamID);
        it->snapshotContext = nullptr;
        return error;
    }

    it->snapshotContext = snapshotBranchBin;

    return CameraError::SUCCESS;
}

// Stop snapshot stream
CameraError CameraDevice::StopSnapshotStream(uint16_t streamID)
{
    std::lock_guard<std::recursive_mutex> lock(mPipelineMutex);
    auto it = std::find_if(mSnapshotStreams.begin(), mSnapshotStreams.end(),
                           [streamID](const SnapshotStream & s) { return s.snapshotStreamParams.snapshotStreamID == streamID; });
    if (it == mSnapshotStreams.end())
    {
        return CameraError::ERROR_SNAPSHOT_STREAM_STOP_FAILED;
    }

    CameraError error              = CameraError::SUCCESS;
    GstElement * snapshotBranchBin = reinterpret_cast<GstElement *>(it->snapshotContext);
    if (snapshotBranchBin != nullptr)
    {
        it->snapshotContext = nullptr;
        error               = DetachStreamBranch(snapshotBranchBin, CameraError::ERROR_SNAPSHOT_STREAM_STOP_FAILED);
    }

    return error;
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
          false,
          chip::MakeOptional(static_cast<bool>(false)) },
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
    static std::vector<StreamUsageEnum> supportedStreamUsage = { StreamUsageEnum::kLiveView, StreamUsageEnum::kRecording,
                                                                 StreamUsageEnum::kAnalysis };
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

std::vector<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> CameraDevice::GetSupportedAmbientContexts()
{
    return kSupportedAmbientContexts;
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

CameraError CameraDevice::SetPrivacyZone(uint16_t zoneId, const TwoDCartesianZoneStorage & zone)
{
    std::lock_guard<std::mutex> lock(mPrivacyZonesMutex);
    mPrivacyZones[zoneId] = zone;
    ++mPrivacyZoneGeneration;
    mMediaController.ClearVideoPreRollBuffer();
    return CameraError::SUCCESS;
}

CameraError CameraDevice::RemovePrivacyZone(uint16_t zoneId)
{
    std::lock_guard<std::mutex> lock(mPrivacyZonesMutex);
    mPrivacyZones.erase(zoneId);
    ++mPrivacyZoneGeneration;
    mMediaController.ClearVideoPreRollBuffer();
    return CameraError::SUCCESS;
}

CameraError CameraDevice::ClearPrivacyZones()
{
    std::lock_guard<std::mutex> lock(mPrivacyZonesMutex);
    mPrivacyZones.clear();
    ++mPrivacyZoneGeneration;
    mMediaController.ClearVideoPreRollBuffer();
    return CameraError::SUCCESS;
}

uint64_t CameraDevice::GetPrivacyZoneGeneration() const
{
    std::lock_guard<std::mutex> lock(mPrivacyZonesMutex);
    return mPrivacyZoneGeneration;
}

void CameraDevice::DistributeVideoIfCurrentGeneration(const uint8_t * data, size_t size, uint16_t videoStreamID, int64_t timestamp,
                                                      uint64_t bufferGen)
{
    std::lock_guard<std::mutex> lock(mPrivacyZonesMutex);
    if (bufferGen < mPrivacyZoneGeneration)
    {
        return;
    }
    mMediaController.DistributeVideo(data, size, videoStreamID, timestamp);
}

bool CameraDevice::ApplyPrivacyMaskToI420Buffer(GstPad * pad, GstBuffer * buffer, uint64_t * outGeneration)
{
    std::vector<std::vector<TwoDCartesianVertexStruct>> activePolygons;
    {
        std::lock_guard<std::mutex> lock(mPrivacyZonesMutex);
        if (outGeneration != nullptr)
        {
            *outGeneration = mPrivacyZoneGeneration;
        }
        if (mPrivacyZones.empty())
        {
            return true;
        }
        activePolygons.reserve(mPrivacyZones.size());
        for (const auto & [zoneId, zone] : mPrivacyZones)
        {
            if (zone.use == ZoneUseEnum::kPrivacy && zone.verticesVector.size() >= 3)
            {
                activePolygons.push_back(zone.verticesVector);
            }
        }
    }

    if (activePolygons.empty())
    {
        return true;
    }

    if (pad == nullptr || buffer == nullptr)
    {
        return false;
    }

    GstCaps * caps = gst_pad_get_current_caps(pad);
    if (caps == nullptr)
    {
        return false;
    }

    GstVideoInfo videoInfo;
    gst_video_info_init(&videoInfo);
    const bool hasValidCapsInfo = (gst_video_info_from_caps(&videoInfo, caps) == TRUE) &&
        (GST_VIDEO_INFO_FORMAT(&videoInfo) == GST_VIDEO_FORMAT_I420) && (GST_VIDEO_INFO_WIDTH(&videoInfo) > 0) &&
        (GST_VIDEO_INFO_HEIGHT(&videoInfo) > 0);
    gst_caps_unref(caps);
    if (!hasValidCapsInfo)
    {
        return false;
    }

    GstVideoFrame frame;
    if (!gst_video_frame_map(&frame, &videoInfo, buffer, GST_MAP_WRITE))
    {
        return false;
    }

    const uint16_t frameWidth  = static_cast<uint16_t>(GST_VIDEO_FRAME_WIDTH(&frame));
    const uint16_t frameHeight = static_cast<uint16_t>(GST_VIDEO_FRAME_HEIGHT(&frame));
    auto * yPlane              = static_cast<uint8_t *>(GST_VIDEO_FRAME_PLANE_DATA(&frame, 0));
    auto * uPlane              = static_cast<uint8_t *>(GST_VIDEO_FRAME_PLANE_DATA(&frame, 1));
    auto * vPlane              = static_cast<uint8_t *>(GST_VIDEO_FRAME_PLANE_DATA(&frame, 2));
    const size_t yStride       = static_cast<size_t>(GST_VIDEO_FRAME_PLANE_STRIDE(&frame, 0));
    const size_t uStride       = static_cast<size_t>(GST_VIDEO_FRAME_PLANE_STRIDE(&frame, 1));
    const size_t vStride       = static_cast<size_t>(GST_VIDEO_FRAME_PLANE_STRIDE(&frame, 2));

    const VideoSensorParamsStruct & sensorParams = GetVideoSensorParams();
    const TwoDCartesianVertexStruct maxCartesian = {
        static_cast<uint16_t>(sensorParams.sensorWidth > 0 ? sensorParams.sensorWidth - 1U : 0),
        static_cast<uint16_t>(sensorParams.sensorHeight > 0 ? sensorParams.sensorHeight - 1U : 0)
    };

    for (const auto & polygon : activePolygons)
    {
        auto scaledPolygon = ZoneGeometry::ScaleVertices(polygon, maxCartesian, frameWidth, frameHeight);
        ZoneGeometry::ApplyPrivacyMaskI420(yPlane, yStride, uPlane, uStride, vPlane, vStride, frameWidth, frameHeight,
                                           scaledPolygon);
    }

    gst_video_frame_unmap(&frame);
    return true;
}

bool CameraDevice::IsValidAnalysisZone(const uint16_t zoneId)
{
    return mZoneManager.IsValidAnalysisZone(zoneId);
}

void CameraDevice::HandleSimulatedZoneTriggeredEvent(const std::vector<uint16_t> & zoneIds)
{
    // Zone events are per-zone - each zone needs its own event notification
    for (const auto & zoneId : zoneIds)
    {
        mZoneManager.OnZoneTriggeredEvent(zoneId, ZoneEventTriggeredReasonEnum::kMotion);
    }
    // Transport trigger is per-motion-event - all zones are passed together
    // This deliberate asymmetry reflects that zone events track individual zone activity
    // while transport triggers coordinate recording across all zones in a single motion event
    mPushAVTransportManager.HandleZoneTrigger(zoneIds);
}

void CameraDevice::HandleSimulatedZoneStoppedEvent(uint16_t zoneId)
{
    mZoneManager.OnZoneStoppedEvent(zoneId, ZoneEventStoppedReasonEnum::kActionStopped);
    // Note: PushAVTransportManager doesn't need zone stopped event currently
}

void CameraDevice::HandleSimulatedAmbientContextTriggeredEvent(uint8_t namespaceId, uint8_t tagId, std::vector<uint16_t> zoneIds,
                                                               uint16_t identifiedContextId)
{
    bool triggeredContextEnabled;

    // The manager only expects ZoneIDs if the feature flag is set, if the flag is set, and the provided set is empty, then
    // set this to Null. We will always be given a vector, it may be empty.
    Optional<app::DataModel::Nullable<std::vector<uint16_t>>> mZoneIds;
    if (GetCameraSupportsPerZoneDetect())
    {
        if (!zoneIds.empty())
        {
            mZoneIds = MakeOptional(app::DataModel::MakeNullable(zoneIds));
        }
        else
        {
            mZoneIds = MakeOptional(app::DataModel::NullNullable);
        }
    }

    mAVAnalysisManager.OnAmbientContextTriggeredEvent(namespaceId, tagId, mZoneIds, identifiedContextId, triggeredContextEnabled);

    // We only want to trigger PushAV if the triggering context has been enabled
    if (triggeredContextEnabled)
    {
        mPushAVTransportManager.HandleAmbientContextTrigger(namespaceId, tagId, zoneIds);
    }
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
                        90 /* Quality */,
                        false /* EncodedPixels */,
                        false /* HardwareEncoder */ },
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
                                        snapshotStreamAllocateArgs.quality, 0 /* RefCount */,
                                        snapshotStreamAllocateArgs.encodedPixels, snapshotStreamAllocateArgs.hardwareEncoder,
                                        snapshotStreamAllocateArgs.watermarkEnabled, snapshotStreamAllocateArgs.OSDEnabled },
                                      false,
                                      nullptr };

    mSnapshotStreams.push_back(snapshotStream);
    return true;
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

AvAnalysisDelegate & CameraDevice::GetAVAnalysisDelegate()
{
    return mAVAnalysisManager;
}

AmbientContextSensing::AmbientContextSensingDelegate & CameraDevice::GetAmbientContextSensingDelegate()
{
    return mAmbientContextSensingManager;
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
