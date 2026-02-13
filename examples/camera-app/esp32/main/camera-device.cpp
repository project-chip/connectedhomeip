/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <lib/support/logging/CHIPLogging.h>

#include "bridge_cmd_defs.h"
#include "webrtc_bridge.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

extern int query_snapshot_command_register_response_handler(void);
static uint8_t s_last_quality = 1;

// Structure to hold snapshot data for queue
struct SnapshotData {
    uint8_t* data;
    size_t len;
    esp_err_t status;
};

// FreeRTOS queue to pass snapshot data from callback to CaptureSnapshot
static QueueHandle_t s_snapshot_queue = nullptr;
static constexpr int SNAPSHOT_QUEUE_SIZE = 1;
static constexpr TickType_t SNAPSHOT_TIMEOUT_MS = pdMS_TO_TICKS(5000); // 5 second timeout

static void snapshot_response_cb(uint32_t cmd_id, uint32_t seq_id, esp_err_t status, uint8_t * resp_data, size_t resp_len)
{
    (void) cmd_id;
    (void) seq_id;

    if (!s_snapshot_queue)
    {
        ESP_LOGE("snapshot_response_cb", "Snapshot queue not initialized");
        if (resp_data) free(resp_data);
        return;
    }

    SnapshotData snapshot;
    snapshot.status = status;

    if (status != ESP_OK)
    {
        ESP_LOGE("snapshot_response_cb", "Snapshot request failed: %s", esp_err_to_name(status));
        snapshot.data = nullptr;
        snapshot.len = 0;
        xQueueSend(s_snapshot_queue, &snapshot, 0);
        if (resp_data) free(resp_data);
        return;
    }

    if (!resp_data || resp_len == 0)
    {
        ESP_LOGE("snapshot_response_cb", "Empty snapshot response");
        snapshot.data = nullptr;
        snapshot.len = 0;
        snapshot.status = ESP_ERR_INVALID_RESPONSE;
        xQueueSend(s_snapshot_queue, &snapshot, 0);
        return;
    }

    if (resp_len >= 2 && resp_data[0] == 0xFF && resp_data[1] == 0xD8)
    {
        ESP_LOGI("snapshot_response_cb", "Valid JPEG snapshot received: %zu bytes", resp_len);
    }
    else
    {
        ESP_LOGW("snapshot_response_cb", "Snapshot data received but JPEG header not found (%zu bytes)", resp_len);
    }

    // Store the snapshot data (don't free it here - it will be freed after use in CaptureSnapshot)
    snapshot.data = resp_data;
    snapshot.len = resp_len;

    // Send to queue (non-blocking, should always succeed since queue size is 1 and we wait in CaptureSnapshot)
    if (xQueueSend(s_snapshot_queue, &snapshot, 0) != pdTRUE)
    {
        ESP_LOGE("snapshot_response_cb", "Failed to send snapshot to queue");
        free(resp_data);
    }
}

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvStreamManagement;
using namespace chip::app::Clusters::WebRTCTransportProvider;

using namespace Camera;

CameraDevice::CameraDevice()
{
    // Set the CameraHALInterface in CameraAVStreamManager
    mCameraAVStreamManager.SetCameraDeviceHAL(this);

    // Set the CameraDevice interface in WebRTCManager
    mWebRTCProviderManager.SetCameraDevice(this);
}

CameraDevice::~CameraDevice() {}

void CameraDevice::Init()
{
    InitializeCameraDevice();
    InitializeStreams();
    mWebRTCProviderManager.Init();
    /* Initialize bridge command framework (bridge is started inside app_webrtc_run) */
    if (bridge_cmd_init() != ESP_OK) {
        ESP_LOGE("CameraDevice::Init", "Failed to initialize bridge command subsystem");
    }

    // Create queue for snapshot data
    if (!s_snapshot_queue)
    {
        s_snapshot_queue = xQueueCreate(SNAPSHOT_QUEUE_SIZE, sizeof(SnapshotData));
        if (!s_snapshot_queue)
        {
            ESP_LOGE("CameraDevice::Init", "Failed to create snapshot queue");
        }
    }

    bridge_cmd_register_response_handler(BRIDGE_CMD_GET_SNAPSHOT, snapshot_response_cb);
}

CameraError CameraDevice::InitializeCameraDevice()
{
    return CameraError::SUCCESS;
}

CameraError CameraDevice::InitializeStreams()
{
    InitializeVideoStreams();
    InitializeAudioStreams();
    InitializeSnapshotStreams();

    return CameraError::SUCCESS;
}

// Find the closest allocated snapshot stream with resolution >= requested, or
// closest possible
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

    ESP_LOGI("CameraDevice::CaptureSnapshot", "Requesting JPEG snapshot from streaming device (quality=%u)...", s_last_quality);

    if (!s_snapshot_queue)
    {
        ESP_LOGE("CameraDevice::CaptureSnapshot", "Snapshot queue not initialized");
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    // Clear any pending data in the queue
    SnapshotData dummy;
    while (xQueueReceive(s_snapshot_queue, &dummy, 0) == pdTRUE)
    {
        if (dummy.data)
        {
            free(dummy.data);
        }
    }

    bridge_cmd_snapshot_req_t req = {
        .quality = s_last_quality,
    };

    esp_err_t err = bridge_cmd_send(BRIDGE_CMD_GET_SNAPSHOT, (const uint8_t *) &req, sizeof(req));

    if (err != ESP_OK)
    {
        ESP_LOGE("CameraDevice::CaptureSnapshot", "Failed to send snapshot request: %s", esp_err_to_name(err));
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    // Wait for snapshot data from callback
    SnapshotData snapshot;
    if (xQueueReceive(s_snapshot_queue, &snapshot, SNAPSHOT_TIMEOUT_MS) != pdTRUE)
    {
        ESP_LOGE("CameraDevice::CaptureSnapshot", "Timeout waiting for snapshot response");
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    // Check if snapshot was successful
    if (snapshot.status != ESP_OK || !snapshot.data || snapshot.len == 0)
    {
        ESP_LOGE("CameraDevice::CaptureSnapshot", "Snapshot failed: status=%s, data=%p, len=%zu",
                 esp_err_to_name(snapshot.status), snapshot.data, snapshot.len);
        if (snapshot.data)
        {
            free(snapshot.data);
        }
        return CameraError::ERROR_CAPTURE_SNAPSHOT_FAILED;
    }

    // Copy the actual snapshot data to the output
    outImageSnapshot.data.assign(snapshot.data, snapshot.data + snapshot.len);

    // Free the snapshot data now that we've copied it
    free(snapshot.data);

    outImageSnapshot.imageRes   = matchedRes;
    outImageSnapshot.imageCodec = matchedCodec;

    ESP_LOGI("CameraDevice::CaptureSnapshot", "Snapshot captured successfully: %zu bytes", snapshot.len);
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
    return false;
}

bool CameraDevice::GetCameraSupportsNightVision()
{
    return false;
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
    return false;
}

bool CameraDevice::GetCameraSupportsImageControl()
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
    static std::vector<RateDistortionTradeOffStruct> rateDistTradeOffs = { {
        VideoCodecEnum::kH264, { kMinResolutionWidth, kMinResolutionHeight }, 10000 /* bitrate */
    } };
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
    static std::vector<SnapshotCapabilitiesStruct> snapshotCapabilities = { { { kMinResolutionWidth, kMinResolutionHeight },
                                                                              kSnapshotStreamFrameRate,
                                                                              ImageCodecEnum::kJpeg,
                                                                              false,
                                                                              chip::MakeOptional(static_cast<bool>(false)) } };
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

void CameraDevice::InitializeVideoStreams()
{
    // Create a video stream with a max resolution of 720p and max frame rate of
    // 60 fps
    VideoStream videoStream1 = { {
                                     1 /* Id */,
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
                                     0 /* RefCount */
                                 },
                                 false,
                                 { mViewport.x1, mViewport.y1, mViewport.x2, mViewport.y2 },
                                 nullptr };
    mVideoStreams.push_back(videoStream1);

    // Create a video stream for the full range(fps, resolution, bitrate)
    // supported by the camera.
    VideoStream videoStream2 = { {
                                     2 /* Id */,
                                     StreamUsageEnum::kLiveView /* StreamUsage */,
                                     VideoCodecEnum::kH264,
                                     kMinVideoFrameRate /* MinFrameRate */,
                                     k60fpsVideoFrameRate /* MaxFrameRate */,
                                     { kMinResolutionWidth, kMinResolutionHeight } /* MinResolution */,
                                     { kMaxResolutionWidth, kMaxResolutionHeight } /* MaxResolution */,
                                     kMinBitRateBps /* MinBitRate */,
                                     kMaxBitRateBps /* MaxBitRate */,
                                     kKeyFrameIntervalMsec /* KeyFrameInterval */,
                                     chip::MakeOptional(static_cast<bool>(false)) /* WMark */,
                                     chip::MakeOptional(static_cast<bool>(false)) /* OSD */,
                                     0 /* RefCount */
                                 },
                                 false,
                                 { mViewport.x1, mViewport.y1, mViewport.x2, mViewport.y2 },
                                 nullptr };

    mVideoStreams.push_back(videoStream2);

    VideoStream videoStream3 = { {
                                     3 /* Id */,
                                     StreamUsageEnum::kLiveView /* StreamUsage */,
                                     VideoCodecEnum::kH264,
                                     kMinVideoFrameRate /* MinFrameRate */,
                                     k60fpsVideoFrameRate /* MaxFrameRate */,
                                     { kMinResolutionWidth, kMinResolutionHeight } /* MinResolution */,
                                     { kMaxResolutionWidth, kMaxResolutionHeight } /* MaxResolution */,
                                     kMinBitRateBps /* MinBitRate */,
                                     kMaxBitRateBps /* MaxBitRate */,
                                     kKeyFrameIntervalMsec /* KeyFrameInterval */,
                                     chip::MakeOptional(static_cast<bool>(false)) /* WMark */,
                                     chip::MakeOptional(static_cast<bool>(false)) /* OSD */,
                                     0 /* RefCount */
                                 },
                                 false,
                                 { mViewport.x1, mViewport.y1, mViewport.x2, mViewport.y2 },
                                 nullptr };

    mVideoStreams.push_back(videoStream3);
}

void CameraDevice::InitializeAudioStreams()
{

    // Mono stream
    AudioStream monoStream = { {
                                   1 /* Id */, StreamUsageEnum::kLiveView, AudioCodecEnum::kOpus, 1 /* ChannelCount: Mono */,
                                   48000 /* SampleRate */, 20000 /* BitRate */, 24 /* BitDepth */, 0 /* RefCount */
                               },
                               false,
                               nullptr };
    mAudioStreams.push_back(monoStream);

    // Stereo stream
    AudioStream stereoStream = { {
                                     2 /* Id */, StreamUsageEnum::kLiveView, AudioCodecEnum::kOpus, 2 /* ChannelCount: Stereo */,
                                     48000 /* SampleRate */, 32000 /* BitRate */, 24 /* BitDepth */, 0 /* RefCount */
                                 },
                                 false,
                                 nullptr };
    mAudioStreams.push_back(stereoStream);

    // Max channel count stream (from spec constant)
    AudioStream maxChannelStream = { {
                                         3 /* Id */, StreamUsageEnum::kLiveView, AudioCodecEnum::kOpus,
                                         kMicrophoneMaxChannelCount /* Max from Spec */, 48000 /* SampleRate */,
                                         64000 /* BitRate */, 24 /* BitDepth */, 0 /* RefCount */
                                     },
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
        ChipLogError(Camera, "Maximum number of snapshot streams reached. Cannot allocate new one");
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

        // Find a unique stream id, starting from the last used one above,
        // incrementing and wrapping at 65535.
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

    SnapshotStream snapshotStream = { {
                                          streamId, snapshotStreamAllocateArgs.imageCodec, snapshotStreamAllocateArgs.maxFrameRate,
                                          snapshotStreamAllocateArgs.minResolution, snapshotStreamAllocateArgs.maxResolution,
                                          snapshotStreamAllocateArgs.quality, 0 /* RefCount */
                                      },
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

CameraAVStreamManagementDelegate & CameraDevice::GetCameraAVStreamMgmtDelegate()
{
    return mCameraAVStreamManager;
}

CameraAVStreamController & CameraDevice::GetCameraAVStreamMgmtController()
{
    return mCameraAVStreamManager;
}
