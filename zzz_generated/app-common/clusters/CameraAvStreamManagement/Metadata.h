// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CameraAvStreamManagement (cluster code: 1361/0x551)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/CameraAvStreamManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvStreamManagement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace MaxConcurrentEncoders {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxConcurrentEncoders::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxConcurrentEncoders
namespace MaxEncodedPixelRate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxEncodedPixelRate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxEncodedPixelRate
namespace VideoSensorParams {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = VideoSensorParams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace VideoSensorParams
namespace NightVisionUsesInfrared {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NightVisionUsesInfrared::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NightVisionUsesInfrared
namespace MinViewport {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinViewport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinViewport
namespace RateDistortionTradeOffPoints {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RateDistortionTradeOffPoints::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RateDistortionTradeOffPoints
namespace MaxContentBufferSize {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxContentBufferSize::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxContentBufferSize
namespace MicrophoneCapabilities {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MicrophoneCapabilities::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MicrophoneCapabilities
namespace SpeakerCapabilities {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SpeakerCapabilities::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SpeakerCapabilities
namespace TwoWayTalkSupport {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TwoWayTalkSupport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TwoWayTalkSupport
namespace SnapshotCapabilities {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SnapshotCapabilities::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SnapshotCapabilities
namespace MaxNetworkBandwidth {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxNetworkBandwidth::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxNetworkBandwidth
namespace CurrentFrameRate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentFrameRate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentFrameRate
namespace HDRModeEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = HDRModeEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace HDRModeEnabled
namespace SupportedStreamUsages {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SupportedStreamUsages::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedStreamUsages
namespace AllocatedVideoStreams {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AllocatedVideoStreams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AllocatedVideoStreams
namespace AllocatedAudioStreams {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AllocatedAudioStreams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AllocatedAudioStreams
namespace AllocatedSnapshotStreams {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AllocatedSnapshotStreams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AllocatedSnapshotStreams
namespace RankedVideoStreamPrioritiesList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RankedVideoStreamPrioritiesList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RankedVideoStreamPrioritiesList
namespace SoftRecordingPrivacyModeEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SoftRecordingPrivacyModeEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace SoftRecordingPrivacyModeEnabled
namespace SoftLivestreamPrivacyModeEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SoftLivestreamPrivacyModeEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace SoftLivestreamPrivacyModeEnabled
namespace HardPrivacyModeOn {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = HardPrivacyModeOn::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace HardPrivacyModeOn
namespace NightVision {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NightVision::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace NightVision
namespace NightVisionIllum {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NightVisionIllum::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace NightVisionIllum
namespace Viewport {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Viewport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace Viewport
namespace SpeakerMuted {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SpeakerMuted::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace SpeakerMuted
namespace SpeakerVolumeLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SpeakerVolumeLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace SpeakerVolumeLevel
namespace SpeakerMaxLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SpeakerMaxLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
} // namespace SpeakerMaxLevel
namespace SpeakerMinLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SpeakerMinLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
} // namespace SpeakerMinLevel
namespace MicrophoneMuted {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MicrophoneMuted::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace MicrophoneMuted
namespace MicrophoneVolumeLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MicrophoneVolumeLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace MicrophoneVolumeLevel
namespace MicrophoneMaxLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MicrophoneMaxLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
} // namespace MicrophoneMaxLevel
namespace MicrophoneMinLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MicrophoneMinLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
} // namespace MicrophoneMinLevel
namespace MicrophoneAGCEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MicrophoneAGCEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace MicrophoneAGCEnabled
namespace ImageRotation {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ImageRotation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ImageRotation
namespace ImageFlipHorizontal {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ImageFlipHorizontal::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ImageFlipHorizontal
namespace ImageFlipVertical {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ImageFlipVertical::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ImageFlipVertical
namespace LocalVideoRecordingEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LocalVideoRecordingEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LocalVideoRecordingEnabled
namespace LocalSnapshotRecordingEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LocalSnapshotRecordingEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LocalSnapshotRecordingEnabled
namespace StatusLightEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = StatusLightEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace StatusLightEnabled
namespace StatusLightBrightness {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = StatusLightBrightness::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace StatusLightBrightness

} // namespace Attributes

namespace Commands {
namespace AudioStreamAllocate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AudioStreamAllocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace AudioStreamAllocate
namespace AudioStreamDeallocate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AudioStreamDeallocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace AudioStreamDeallocate
namespace VideoStreamAllocate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = VideoStreamAllocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace VideoStreamAllocate
namespace VideoStreamModify {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = VideoStreamModify::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace VideoStreamModify
namespace VideoStreamDeallocate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = VideoStreamDeallocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace VideoStreamDeallocate
namespace SnapshotStreamAllocate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SnapshotStreamAllocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace SnapshotStreamAllocate
namespace SnapshotStreamModify {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SnapshotStreamModify::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace SnapshotStreamModify
namespace SnapshotStreamDeallocate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SnapshotStreamDeallocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace SnapshotStreamDeallocate
namespace SetStreamPriorities {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetStreamPriorities::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SetStreamPriorities
namespace CaptureSnapshot {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CaptureSnapshot::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace CaptureSnapshot

} // namespace Commands
} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
