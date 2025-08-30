// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CameraAvStreamManagement (cluster code: 1361/0x551)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/CameraAvStreamManagement/Ids.h>
#include <clusters/CameraAvStreamManagement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::CameraAvStreamManagement::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::CameraAvStreamManagement::Attributes;
        switch (attributeId)
        {
        case MaxConcurrentEncoders::Id:
            return MaxConcurrentEncoders::kMetadataEntry;
        case MaxEncodedPixelRate::Id:
            return MaxEncodedPixelRate::kMetadataEntry;
        case VideoSensorParams::Id:
            return VideoSensorParams::kMetadataEntry;
        case NightVisionUsesInfrared::Id:
            return NightVisionUsesInfrared::kMetadataEntry;
        case MinViewportResolution::Id:
            return MinViewportResolution::kMetadataEntry;
        case RateDistortionTradeOffPoints::Id:
            return RateDistortionTradeOffPoints::kMetadataEntry;
        case MaxContentBufferSize::Id:
            return MaxContentBufferSize::kMetadataEntry;
        case MicrophoneCapabilities::Id:
            return MicrophoneCapabilities::kMetadataEntry;
        case SpeakerCapabilities::Id:
            return SpeakerCapabilities::kMetadataEntry;
        case TwoWayTalkSupport::Id:
            return TwoWayTalkSupport::kMetadataEntry;
        case SnapshotCapabilities::Id:
            return SnapshotCapabilities::kMetadataEntry;
        case MaxNetworkBandwidth::Id:
            return MaxNetworkBandwidth::kMetadataEntry;
        case CurrentFrameRate::Id:
            return CurrentFrameRate::kMetadataEntry;
        case HDRModeEnabled::Id:
            return HDRModeEnabled::kMetadataEntry;
        case SupportedStreamUsages::Id:
            return SupportedStreamUsages::kMetadataEntry;
        case AllocatedVideoStreams::Id:
            return AllocatedVideoStreams::kMetadataEntry;
        case AllocatedAudioStreams::Id:
            return AllocatedAudioStreams::kMetadataEntry;
        case AllocatedSnapshotStreams::Id:
            return AllocatedSnapshotStreams::kMetadataEntry;
        case StreamUsagePriorities::Id:
            return StreamUsagePriorities::kMetadataEntry;
        case SoftRecordingPrivacyModeEnabled::Id:
            return SoftRecordingPrivacyModeEnabled::kMetadataEntry;
        case SoftLivestreamPrivacyModeEnabled::Id:
            return SoftLivestreamPrivacyModeEnabled::kMetadataEntry;
        case HardPrivacyModeOn::Id:
            return HardPrivacyModeOn::kMetadataEntry;
        case NightVision::Id:
            return NightVision::kMetadataEntry;
        case NightVisionIllum::Id:
            return NightVisionIllum::kMetadataEntry;
        case Viewport::Id:
            return Viewport::kMetadataEntry;
        case SpeakerMuted::Id:
            return SpeakerMuted::kMetadataEntry;
        case SpeakerVolumeLevel::Id:
            return SpeakerVolumeLevel::kMetadataEntry;
        case SpeakerMaxLevel::Id:
            return SpeakerMaxLevel::kMetadataEntry;
        case SpeakerMinLevel::Id:
            return SpeakerMinLevel::kMetadataEntry;
        case MicrophoneMuted::Id:
            return MicrophoneMuted::kMetadataEntry;
        case MicrophoneVolumeLevel::Id:
            return MicrophoneVolumeLevel::kMetadataEntry;
        case MicrophoneMaxLevel::Id:
            return MicrophoneMaxLevel::kMetadataEntry;
        case MicrophoneMinLevel::Id:
            return MicrophoneMinLevel::kMetadataEntry;
        case MicrophoneAGCEnabled::Id:
            return MicrophoneAGCEnabled::kMetadataEntry;
        case ImageRotation::Id:
            return ImageRotation::kMetadataEntry;
        case ImageFlipHorizontal::Id:
            return ImageFlipHorizontal::kMetadataEntry;
        case ImageFlipVertical::Id:
            return ImageFlipVertical::kMetadataEntry;
        case LocalVideoRecordingEnabled::Id:
            return LocalVideoRecordingEnabled::kMetadataEntry;
        case LocalSnapshotRecordingEnabled::Id:
            return LocalSnapshotRecordingEnabled::kMetadataEntry;
        case StatusLightEnabled::Id:
            return StatusLightEnabled::kMetadataEntry;
        case StatusLightBrightness::Id:
            return StatusLightBrightness::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::CameraAvStreamManagement::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::CameraAvStreamManagement::Commands;
        switch (commandId)
        {
        case AudioStreamAllocate::Id:
            return AudioStreamAllocate::kMetadataEntry;
        case AudioStreamDeallocate::Id:
            return AudioStreamDeallocate::kMetadataEntry;
        case VideoStreamAllocate::Id:
            return VideoStreamAllocate::kMetadataEntry;
        case VideoStreamModify::Id:
            return VideoStreamModify::kMetadataEntry;
        case VideoStreamDeallocate::Id:
            return VideoStreamDeallocate::kMetadataEntry;
        case SnapshotStreamAllocate::Id:
            return SnapshotStreamAllocate::kMetadataEntry;
        case SnapshotStreamModify::Id:
            return SnapshotStreamModify::kMetadataEntry;
        case SnapshotStreamDeallocate::Id:
            return SnapshotStreamDeallocate::kMetadataEntry;
        case SetStreamPriorities::Id:
            return SetStreamPriorities::kMetadataEntry;
        case CaptureSnapshot::Id:
            return CaptureSnapshot::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
