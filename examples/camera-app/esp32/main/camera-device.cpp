#include "camera-device.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>
#include <limits.h>
#include <sys/ioctl.h>

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

    // Create a dummy JPEG image
    static const uint8_t dummy_jpeg[] = {
        0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01, 0x01, 0x01, 0x00, 0x48, 0x00, 0x48, 0x00,
        0x00, 0xFF, 0xDB, 0x00, 0x43, 0x00, 0x08, 0x06, 0x06, 0x07, 0x06, 0x05, 0x08, 0x07, 0x07, 0x07, 0x09, 0x09, 0x08,
        0x0A, 0x0C, 0x14, 0x0D, 0x0C, 0x0B, 0x0B, 0x0C, 0x19, 0x12, 0x13, 0x0F, 0x14, 0x1D, 0x1A, 0x1F, 0x1E, 0x1D, 0x1A,
        0x1C, 0x1C, 0x20, 0x24, 0x2E, 0x27, 0x20, 0x22, 0x2C, 0x23, 0x1C, 0x1C, 0x28, 0x37, 0x29, 0x2C, 0x30, 0x31, 0x34,
        0x34, 0x34, 0x1F, 0x27, 0x39, 0x3D, 0x38, 0x32, 0x3C, 0x2E, 0x33, 0x34, 0x32, 0xFF, 0xC0, 0x00, 0x11, 0x08, 0x00,
        0x20, 0x00, 0x20, 0x01, 0x11, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01, 0xFF, 0xC4, 0x00, 0x14, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xFF, 0xC4, 0x00,
        0x14, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xDA, 0x00, 0x0C, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3F, 0x00, 0x80, 0xFF, 0xD9
    };

    // Copy the dummy JPEG data to the output
    outImageSnapshot.data.assign(dummy_jpeg, dummy_jpeg + sizeof(dummy_jpeg));

    outImageSnapshot.imageRes   = matchedRes;
    outImageSnapshot.imageCodec = matchedCodec;

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
        ChipLogError(Camera,"Maximum number of snapshot streams reached. Cannot allocate new one");
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
