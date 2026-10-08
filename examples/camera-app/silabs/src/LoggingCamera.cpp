/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "LoggingCamera.h"

#include <clusters/WebRTCTransportRequestor/ClusterId.h>
#include <devices/Types.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>

#include <algorithm>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvStreamManagement;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {

namespace {

// Advertised capabilities of the (non-existent) camera sensor.
constexpr uint16_t kSensorWidth          = 1920;
constexpr uint16_t kSensorHeight         = 1080;
constexpr uint16_t kMaxFrameRate         = 30;
constexpr uint16_t kMinViewportWidth     = 640;
constexpr uint16_t kMinViewportHeight    = 360;
constexpr uint16_t kSnapshotWidth        = 640;
constexpr uint16_t kSnapshotHeight       = 480;
constexpr uint8_t kMaxConcurrentEncoders = 1;
constexpr uint32_t kMaxEncodedPixelRate  = static_cast<uint32_t>(kSensorWidth) * kSensorHeight * kMaxFrameRate;
constexpr uint32_t kMaxContentBufferSize = 1024 * 1024;
constexpr uint32_t kMaxNetworkBandwidth  = 10 * 1000 * 1000; // bps
constexpr uint32_t kMinH264BitRate       = 10000;
constexpr uint8_t kMicrophoneChannels    = 1;

const AudioCodecEnum kAudioCodecs[]            = { AudioCodecEnum::kOpus };
const uint32_t kAudioSampleRates[]             = { 48000 };
const uint8_t kAudioBitDepths[]                = { 16 };
const ClusterId kClientClusters[]              = { WebRTCTransportRequestor::Id };
const Globals::StreamUsageEnum kStreamUsages[] = { Globals::StreamUsageEnum::kLiveView, Globals::StreamUsageEnum::kRecording };

template <typename StreamList, typename IdGetter>
uint16_t NextStreamId(const StreamList & streams, IdGetter idGetter)
{
    uint16_t nextId = 0;
    for (const auto & stream : streams)
    {
        nextId = std::max<uint16_t>(nextId, static_cast<uint16_t>(idGetter(stream) + 1));
    }
    return nextId;
}

template <typename StreamList, typename IdGetter>
bool ContainsStream(const StreamList & streams, uint16_t streamId, IdGetter idGetter)
{
    return std::any_of(streams.begin(), streams.end(), [&](const auto & stream) { return idGetter(stream) == streamId; });
}

uint16_t VideoStreamId(const VideoStreamStruct & stream)
{
    return stream.videoStreamID;
}

uint16_t AudioStreamId(const AudioStreamStruct & stream)
{
    return stream.audioStreamID;
}

uint16_t SnapshotStreamId(const SnapshotStreamStruct & stream)
{
    return stream.snapshotStreamID;
}

} // namespace

LoggingCamera::LoggingCamera() : SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kCamera, 1)) {}

CHIP_ERROR LoggingCamera::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    BitFlags<Feature> features(Feature::kVideo, Feature::kAudio, Feature::kSnapshot);

    AudioCapabilitiesStruct microphoneCapabilities;
    microphoneCapabilities.maxNumberOfChannels  = kMicrophoneChannels;
    microphoneCapabilities.supportedCodecs      = Span<const AudioCodecEnum>(kAudioCodecs);
    microphoneCapabilities.supportedSampleRates = Span<const uint32_t>(kAudioSampleRates);
    microphoneCapabilities.supportedBitDepths   = Span<const uint8_t>(kAudioBitDepths);

    SnapshotCapabilitiesStruct snapshotCapabilities;
    snapshotCapabilities.resolution              = { kSnapshotWidth, kSnapshotHeight };
    snapshotCapabilities.maxFrameRate            = kMaxFrameRate;
    snapshotCapabilities.imageCodec              = ImageCodecEnum::kJpeg;
    snapshotCapabilities.requiresEncodedPixels   = false;
    snapshotCapabilities.requiresHardwareEncoder = MakeOptional(false);

    RateDistortionTradeOffStruct rateDistortionTradeOffPoint;
    rateDistortionTradeOffPoint.codec      = VideoCodecEnum::kH264;
    rateDistortionTradeOffPoint.resolution = { kMinViewportWidth, kMinViewportHeight };
    rateDistortionTradeOffPoint.minBitRate = kMinH264BitRate;

    VideoSensorParamsStruct sensorParams;
    sensorParams.sensorWidth  = kSensorWidth;
    sensorParams.sensorHeight = kSensorHeight;
    sensorParams.maxFPS       = kMaxFrameRate;

    CameraAVStreamManagementCluster::InitArguments args{
        .delegate                     = *this,
        .endpointId                   = endpoint,
        .features                     = features,
        .optionalAttrs                = BitFlags<OptionalAttribute>(),
        .maxConcurrentEncoders        = kMaxConcurrentEncoders,
        .maxEncodedPixelRate          = kMaxEncodedPixelRate,
        .videoSensorParams            = sensorParams,
        .nightVisionUsesInfrared      = false,
        .minViewPort                  = { kMinViewportWidth, kMinViewportHeight },
        .rateDistortionTradeOffPoints = { rateDistortionTradeOffPoint },
        .maxContentBufferSize         = kMaxContentBufferSize,
        .microphoneCapabilities       = microphoneCapabilities,
        .spkrCapabilities             = AudioCapabilitiesStruct(),
        .twoWayTalkSupport            = TwoWayTalkSupportTypeEnum::kNotSupported,
        .snapshotCapabilities         = { snapshotCapabilities },
        .maxNetworkBandwidth          = kMaxNetworkBandwidth,
        .supportedStreamUsages        = std::vector<Globals::StreamUsageEnum>(std::begin(kStreamUsages), std::end(kStreamUsages)),
        .streamUsagePriorities        = std::vector<Globals::StreamUsageEnum>(std::begin(kStreamUsages), std::end(kStreamUsages)),
    };
    mAvStreamManagementCluster.Create(std::move(args));
    ReturnErrorOnFailure(provider.AddCluster(mAvStreamManagementCluster.Registration()));

    mWebRTCTransportProviderCluster.Create(endpoint, *this);
    ReturnErrorOnFailure(provider.AddCluster(mWebRTCTransportProviderCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void LoggingCamera::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mWebRTCTransportProviderCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mWebRTCTransportProviderCluster.Cluster()));
        mWebRTCTransportProviderCluster.Destroy();
    }
    if (mAvStreamManagementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mAvStreamManagementCluster.Cluster()));
        mAvStreamManagementCluster.Destroy();
    }
    mSessionStreams.clear();
}

CHIP_ERROR LoggingCamera::ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const
{
    return out.ReferenceExisting(Span<const ClusterId>(kClientClusters));
}

CHIP_ERROR LoggingCamera::InitClusters()
{
    VerifyOrReturnError(mAvStreamManagementCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);

    auto & cluster = mAvStreamManagementCluster.Cluster();
    ReturnErrorOnFailure(cluster.SetViewport({ 0, 0, kSensorWidth, kSensorHeight }));
    ReturnErrorOnFailure(cluster.Init());

    ChipLogProgress(Camera, "LoggingCamera[ep=%u]: clusters initialized (no camera hardware attached)", mEndpointId);
    return CHIP_NO_ERROR;
}

// ---------------------------------------------------------------------------
// CameraAVStreamManagementDelegate
// ---------------------------------------------------------------------------

Status LoggingCamera::VideoStreamAllocate(const VideoStreamStruct & allocateArgs, uint16_t & outStreamID)
{
    VerifyOrReturnValue(mAvStreamManagementCluster.IsConstructed(), Status::Failure);
    auto & cluster = mAvStreamManagementCluster.Cluster();

    std::optional<uint16_t> reusableId = cluster.GetReusableVideoStreamId(allocateArgs);
    if (reusableId.has_value())
    {
        outStreamID = reusableId.value();
    }
    else
    {
        VerifyOrReturnValue(cluster.GetAllocatedVideoStreams().size() < CHIP_CONFIG_MAX_NUM_CAMERA_VIDEO_STREAMS,
                            Status::ResourceExhausted);
        outStreamID = NextStreamId(cluster.GetAllocatedVideoStreams(), VideoStreamId);
    }

    ChipLogProgress(Camera, "LoggingCamera: VideoStreamAllocate usage=%u codec=%u %ux%u-%ux%u @%u-%u fps -> id=%u",
                    to_underlying(allocateArgs.streamUsage), to_underlying(allocateArgs.videoCodec),
                    allocateArgs.minResolution.width, allocateArgs.minResolution.height, allocateArgs.maxResolution.width,
                    allocateArgs.maxResolution.height, allocateArgs.minFrameRate, allocateArgs.maxFrameRate, outStreamID);
    return Status::Success;
}

void LoggingCamera::OnVideoStreamAllocated(const VideoStreamStruct & allocatedStream, StreamAllocationAction action)
{
    ChipLogProgress(Camera, "LoggingCamera: OnVideoStreamAllocated id=%u action=%u (no video pipeline to start)",
                    allocatedStream.videoStreamID, to_underlying(action));
}

Status LoggingCamera::VideoStreamModify(const uint16_t streamID, const Optional<bool> waterMarkEnabled,
                                        const Optional<bool> osdEnabled)
{
    ChipLogProgress(Camera, "LoggingCamera: VideoStreamModify id=%u watermark=%d osd=%d", streamID,
                    waterMarkEnabled.HasValue() ? waterMarkEnabled.Value() : -1, osdEnabled.HasValue() ? osdEnabled.Value() : -1);
    return Status::Success;
}

Status LoggingCamera::VideoStreamDeallocate(const uint16_t streamID)
{
    ChipLogProgress(Camera, "LoggingCamera: VideoStreamDeallocate id=%u", streamID);
    return Status::Success;
}

Status LoggingCamera::AudioStreamAllocate(const AudioStreamStruct & allocateArgs, uint16_t & outStreamID)
{
    VerifyOrReturnValue(mAvStreamManagementCluster.IsConstructed(), Status::Failure);
    const auto & streams = mAvStreamManagementCluster.Cluster().GetAllocatedAudioStreams();
    VerifyOrReturnValue(streams.size() < CHIP_CONFIG_MAX_NUM_CAMERA_AUDIO_STREAMS, Status::ResourceExhausted);

    outStreamID = NextStreamId(streams, AudioStreamId);
    ChipLogProgress(Camera, "LoggingCamera: AudioStreamAllocate usage=%u codec=%u channels=%u rate=%lu -> id=%u",
                    to_underlying(allocateArgs.streamUsage), to_underlying(allocateArgs.audioCodec), allocateArgs.channelCount,
                    static_cast<unsigned long>(allocateArgs.sampleRate), outStreamID);
    return Status::Success;
}

Status LoggingCamera::AudioStreamDeallocate(const uint16_t streamID)
{
    ChipLogProgress(Camera, "LoggingCamera: AudioStreamDeallocate id=%u", streamID);
    return Status::Success;
}

Status LoggingCamera::SnapshotStreamAllocate(const SnapshotStreamAllocateArgs & allocateArgs, uint16_t & outStreamID)
{
    VerifyOrReturnValue(mAvStreamManagementCluster.IsConstructed(), Status::Failure);
    auto & cluster = mAvStreamManagementCluster.Cluster();

    std::optional<uint16_t> reusableId = cluster.GetReusableSnapshotStreamId(allocateArgs);
    if (reusableId.has_value())
    {
        outStreamID = reusableId.value();
    }
    else
    {
        VerifyOrReturnValue(cluster.GetAllocatedSnapshotStreams().size() < CHIP_CONFIG_MAX_NUM_CAMERA_SNAPSHOT_STREAMS,
                            Status::ResourceExhausted);
        outStreamID = NextStreamId(cluster.GetAllocatedSnapshotStreams(), SnapshotStreamId);
    }

    ChipLogProgress(Camera, "LoggingCamera: SnapshotStreamAllocate codec=%u %ux%u-%ux%u quality=%u -> id=%u",
                    to_underlying(allocateArgs.imageCodec), allocateArgs.minResolution.width, allocateArgs.minResolution.height,
                    allocateArgs.maxResolution.width, allocateArgs.maxResolution.height, allocateArgs.quality, outStreamID);
    return Status::Success;
}

Status LoggingCamera::SnapshotStreamModify(const uint16_t streamID, const Optional<bool> waterMarkEnabled,
                                           const Optional<bool> osdEnabled)
{
    ChipLogProgress(Camera, "LoggingCamera: SnapshotStreamModify id=%u watermark=%d osd=%d", streamID,
                    waterMarkEnabled.HasValue() ? waterMarkEnabled.Value() : -1, osdEnabled.HasValue() ? osdEnabled.Value() : -1);
    return Status::Success;
}

Status LoggingCamera::SnapshotStreamDeallocate(const uint16_t streamID)
{
    ChipLogProgress(Camera, "LoggingCamera: SnapshotStreamDeallocate id=%u", streamID);
    return Status::Success;
}

void LoggingCamera::OnStreamUsagePrioritiesChanged()
{
    ChipLogProgress(Camera, "LoggingCamera: StreamUsagePriorities changed");
}

void LoggingCamera::OnAttributeChanged(AttributeId attributeId)
{
    ChipLogProgress(Camera, "LoggingCamera: CameraAvStreamManagement attribute 0x%08lx changed",
                    static_cast<unsigned long>(attributeId));
}

Status LoggingCamera::CaptureSnapshot(const DataModel::Nullable<uint16_t> streamID, const VideoResolutionStruct & resolution,
                                      ImageSnapshot & /* outImageSnapshot */)
{
    ChipLogProgress(Camera, "LoggingCamera: CaptureSnapshot stream=%d %ux%u rejected, no camera attached",
                    streamID.IsNull() ? -1 : static_cast<int>(streamID.Value()), resolution.width, resolution.height);
    return Status::Failure;
}

CHIP_ERROR LoggingCamera::PersistentAttributesLoadedCallback()
{
    ChipLogProgress(Camera, "LoggingCamera: persisted streams loaded (video=%u audio=%u snapshot=%u)",
                    static_cast<unsigned>(mAvStreamManagementCluster.Cluster().GetAllocatedVideoStreams().size()),
                    static_cast<unsigned>(mAvStreamManagementCluster.Cluster().GetAllocatedAudioStreams().size()),
                    static_cast<unsigned>(mAvStreamManagementCluster.Cluster().GetAllocatedSnapshotStreams().size()));
    return CHIP_NO_ERROR;
}

const std::vector<VideoStreamStruct> & LoggingCamera::GetAllocatedVideoStreams() const
{
    static const std::vector<VideoStreamStruct> kEmpty;
    return mAvStreamManagementCluster.IsConstructed() ? mAvStreamManagementCluster.Cluster().GetAllocatedVideoStreams() : kEmpty;
}

const std::vector<AudioStreamStruct> & LoggingCamera::GetAllocatedAudioStreams() const
{
    static const std::vector<AudioStreamStruct> kEmpty;
    return mAvStreamManagementCluster.IsConstructed() ? mAvStreamManagementCluster.Cluster().GetAllocatedAudioStreams() : kEmpty;
}

// ---------------------------------------------------------------------------
// WebRTCTransportProvider::Delegate
// ---------------------------------------------------------------------------

void LoggingCamera::FillSession(const OfferRequestArgs & args, WebRTCSessionStruct & outSession)
{
    outSession.id             = args.sessionId;
    outSession.peerNodeID     = args.peerNodeId;
    outSession.peerEndpointID = args.originatingEndpointId;
    outSession.streamUsage    = args.streamUsage;
    outSession.fabricIndex    = args.fabricIndex;

    // Release references held by a previous negotiation on the same session before re-acquiring.
    ReleaseSessionStreams(args.sessionId);

    SessionStreams & streams = mSessionStreams[args.sessionId];
    if (args.videoStreams.HasValue())
    {
        streams.videoStreams = args.videoStreams.Value();
    }
    if (args.audioStreams.HasValue())
    {
        streams.audioStreams = args.audioStreams.Value();
    }

    auto & cluster = mAvStreamManagementCluster.Cluster();
    for (uint16_t id : streams.videoStreams)
    {
        LogErrorOnFailure(cluster.UpdateVideoStreamRefCount(id, true));
    }
    for (uint16_t id : streams.audioStreams)
    {
        LogErrorOnFailure(cluster.UpdateAudioStreamRefCount(id, true));
    }

    if (streams.videoStreams.empty())
    {
        outSession.videoStreamID.SetNull();
    }
    else
    {
        outSession.videoStreamID.SetNonNull(streams.videoStreams.front());
        outSession.videoStreams.SetValue(DataModel::List<const uint16_t>(streams.videoStreams.data(), streams.videoStreams.size()));
    }

    if (streams.audioStreams.empty())
    {
        outSession.audioStreamID.SetNull();
    }
    else
    {
        outSession.audioStreamID.SetNonNull(streams.audioStreams.front());
        outSession.audioStreams.SetValue(DataModel::List<const uint16_t>(streams.audioStreams.data(), streams.audioStreams.size()));
    }
}

void LoggingCamera::ReleaseSessionStreams(uint16_t sessionId)
{
    auto it = mSessionStreams.find(sessionId);
    VerifyOrReturn(it != mSessionStreams.end());

    if (mAvStreamManagementCluster.IsConstructed())
    {
        auto & cluster = mAvStreamManagementCluster.Cluster();
        for (uint16_t id : it->second.videoStreams)
        {
            LogErrorOnFailure(cluster.UpdateVideoStreamRefCount(id, false));
        }
        for (uint16_t id : it->second.audioStreams)
        {
            LogErrorOnFailure(cluster.UpdateAudioStreamRefCount(id, false));
        }
    }
    mSessionStreams.erase(it);
}

CHIP_ERROR LoggingCamera::HandleSolicitOffer(const OfferRequestArgs & args, WebRTCSessionStruct & outSession,
                                             bool & outDeferredOffer)
{
    VerifyOrReturnError(mAvStreamManagementCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);

    FillSession(args, outSession);
    outDeferredOffer = false;

    ChipLogProgress(Camera,
                    "LoggingCamera: SolicitOffer session=%u peer=" ChipLogFormatX64
                    " usage=%u. No camera attached, no SDP Offer will be sent.",
                    args.sessionId, ChipLogValueX64(args.peerNodeId), to_underlying(args.streamUsage));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingCamera::HandleProvideOffer(const ProvideOfferRequestArgs & args, WebRTCSessionStruct & outSession)
{
    VerifyOrReturnError(mAvStreamManagementCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);

    FillSession(args, outSession);

    ChipLogProgress(Camera,
                    "LoggingCamera: ProvideOffer session=%u peer=" ChipLogFormatX64
                    " usage=%u sdpLen=%u. No camera attached, no SDP Answer will be sent.",
                    args.sessionId, ChipLogValueX64(args.peerNodeId), to_underlying(args.streamUsage),
                    static_cast<unsigned>(args.sdp.size()));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingCamera::HandleProvideAnswer(uint16_t sessionId, const std::string & sdpAnswer)
{
    ChipLogProgress(Camera, "LoggingCamera: ProvideAnswer session=%u sdpLen=%u", sessionId,
                    static_cast<unsigned>(sdpAnswer.size()));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingCamera::HandleProvideICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates)
{
    ChipLogProgress(Camera, "LoggingCamera: ProvideICECandidates session=%u count=%u", sessionId,
                    static_cast<unsigned>(candidates.size()));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingCamera::HandleEndSession(uint16_t sessionId, WebRTCEndReasonEnum reasonCode)
{
    ChipLogProgress(Camera, "LoggingCamera: EndSession session=%u reason=%u", sessionId, to_underlying(reasonCode));
    ReleaseSessionStreams(sessionId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingCamera::ValidateStreamUsage(StreamUsageEnum streamUsage, Optional<std::vector<uint16_t>> & videoStreams,
                                              Optional<std::vector<uint16_t>> & audioStreams)
{
    VerifyOrReturnError(mAvStreamManagementCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(IsStreamUsageSupported(streamUsage));

    auto & cluster = mAvStreamManagementCluster.Cluster();

    // Empty lists request automatic selection: prefer a stream with the same usage, else any allocated stream.
    if (videoStreams.HasValue() && videoStreams.Value().empty() && !cluster.GetAllocatedVideoStreams().empty())
    {
        const auto & allocated = cluster.GetAllocatedVideoStreams();
        auto match = std::find_if(allocated.begin(), allocated.end(), [&](const auto & s) { return s.streamUsage == streamUsage; });
        videoStreams.Value().push_back(match != allocated.end() ? match->videoStreamID : allocated.front().videoStreamID);
    }

    if (audioStreams.HasValue() && audioStreams.Value().empty() && !cluster.GetAllocatedAudioStreams().empty())
    {
        const auto & allocated = cluster.GetAllocatedAudioStreams();
        auto match = std::find_if(allocated.begin(), allocated.end(), [&](const auto & s) { return s.streamUsage == streamUsage; });
        audioStreams.Value().push_back(match != allocated.end() ? match->audioStreamID : allocated.front().audioStreamID);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingCamera::ValidateVideoStreamID(uint16_t videoStreamId)
{
    return ContainsStream(GetAllocatedVideoStreams(), videoStreamId, VideoStreamId) ? CHIP_NO_ERROR : CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR LoggingCamera::ValidateAudioStreamID(uint16_t audioStreamId)
{
    return ContainsStream(GetAllocatedAudioStreams(), audioStreamId, AudioStreamId) ? CHIP_NO_ERROR : CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR LoggingCamera::ValidateVideoStreams(const std::vector<uint16_t> & videoStreams)
{
    for (uint16_t id : videoStreams)
    {
        ReturnErrorOnFailure(ValidateVideoStreamID(id));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingCamera::ValidateAudioStreams(const std::vector<uint16_t> & audioStreams)
{
    for (uint16_t id : audioStreams)
    {
        ReturnErrorOnFailure(ValidateAudioStreamID(id));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingCamera::IsStreamUsageSupported(StreamUsageEnum streamUsage)
{
    VerifyOrReturnError(mAvStreamManagementCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    const auto & priorities = mAvStreamManagementCluster.Cluster().GetStreamUsagePriorities();
    return (std::find(priorities.begin(), priorities.end(), streamUsage) != priorities.end()) ? CHIP_NO_ERROR
                                                                                              : CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR LoggingCamera::IsHardPrivacyModeActive(bool & isActive)
{
    VerifyOrReturnError(mAvStreamManagementCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    isActive = mAvStreamManagementCluster.Cluster().GetHardPrivacyModeOn();
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingCamera::IsSoftRecordingPrivacyModeActive(bool & isActive)
{
    VerifyOrReturnError(mAvStreamManagementCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    isActive = mAvStreamManagementCluster.Cluster().GetSoftRecordingPrivacyModeEnabled();
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingCamera::IsSoftLivestreamPrivacyModeActive(bool & isActive)
{
    VerifyOrReturnError(mAvStreamManagementCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    isActive = mAvStreamManagementCluster.Cluster().GetSoftLivestreamPrivacyModeEnabled();
    return CHIP_NO_ERROR;
}

bool LoggingCamera::HasAllocatedVideoStreams()
{
    return !GetAllocatedVideoStreams().empty();
}

bool LoggingCamera::HasAllocatedAudioStreams()
{
    return !GetAllocatedAudioStreams().empty();
}

CHIP_ERROR LoggingCamera::ValidateSFrameConfig(uint16_t cipherSuite, size_t baseKeyLength)
{
    // SFrame cipher suites: 0x0001 AES-128-GCM-SHA256 (16 byte key), 0x0002 AES-256-GCM-SHA512 (32 byte key).
    switch (cipherSuite)
    {
    case 0x0001:
        return (baseKeyLength == 16) ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT;
    case 0x0002:
        return (baseKeyLength == 32) ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT;
    default:
        ChipLogError(Camera, "LoggingCamera: unsupported SFrame cipher suite 0x%04x", cipherSuite);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

CHIP_ERROR LoggingCamera::IsUTCTimeNull(bool & isNull)
{
    // No Time Synchronization cluster on this device; do not block TURNS/STUNS ICE servers.
    isNull = false;
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
