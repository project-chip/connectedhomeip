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
namespace MaxConcurrentVideoEncoders {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MaxConcurrentVideoEncoders::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxConcurrentVideoEncoders
namespace MaxEncodedPixelRate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MaxEncodedPixelRate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxEncodedPixelRate
namespace VideoSensorParams {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::VideoSensorParams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace VideoSensorParams
namespace NightVisionCapable {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::NightVisionCapable::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NightVisionCapable
namespace MinViewport {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MinViewport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinViewport
namespace RateDistortionTradeOffPoints {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::RateDistortionTradeOffPoints::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RateDistortionTradeOffPoints
namespace MaxContentBufferSize {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MaxContentBufferSize::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxContentBufferSize
namespace MicrophoneCapabilities {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MicrophoneCapabilities::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MicrophoneCapabilities
namespace SpeakerCapabilities {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SpeakerCapabilities::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SpeakerCapabilities
namespace TwoWayTalkSupport {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::TwoWayTalkSupport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TwoWayTalkSupport
namespace SupportedSnapshotParams {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SupportedSnapshotParams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedSnapshotParams
namespace MaxNetworkBandwidth {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MaxNetworkBandwidth::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxNetworkBandwidth
namespace CurrentFrameRate {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::CurrentFrameRate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentFrameRate
namespace HDRModeEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::HDRModeEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace HDRModeEnabled
namespace SupportedStreamUsages {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SupportedStreamUsages::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedStreamUsages
namespace AllocatedVideoStreams {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::AllocatedVideoStreams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AllocatedVideoStreams
namespace AllocatedAudioStreams {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::AllocatedAudioStreams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AllocatedAudioStreams
namespace AllocatedSnapshotStreams {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::AllocatedSnapshotStreams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AllocatedSnapshotStreams
namespace RankedVideoStreamPrioritiesList {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::RankedVideoStreamPrioritiesList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RankedVideoStreamPrioritiesList
namespace SoftRecordingPrivacyModeEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SoftRecordingPrivacyModeEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace SoftRecordingPrivacyModeEnabled
namespace SoftLivestreamPrivacyModeEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SoftLivestreamPrivacyModeEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace SoftLivestreamPrivacyModeEnabled
namespace HardPrivacyModeOn {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::HardPrivacyModeOn::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace HardPrivacyModeOn
namespace NightVision {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::NightVision::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace NightVision
namespace NightVisionIllum {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::NightVisionIllum::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace NightVisionIllum
namespace Viewport {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::Viewport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace Viewport
namespace SpeakerMuted {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SpeakerMuted::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace SpeakerMuted
namespace SpeakerVolumeLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SpeakerVolumeLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace SpeakerVolumeLevel
namespace SpeakerMaxLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SpeakerMaxLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
} // namespace SpeakerMaxLevel
namespace SpeakerMinLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SpeakerMinLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
} // namespace SpeakerMinLevel
namespace MicrophoneMuted {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MicrophoneMuted::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace MicrophoneMuted
namespace MicrophoneVolumeLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MicrophoneVolumeLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace MicrophoneVolumeLevel
namespace MicrophoneMaxLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MicrophoneMaxLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
} // namespace MicrophoneMaxLevel
namespace MicrophoneMinLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MicrophoneMinLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
} // namespace MicrophoneMinLevel
namespace MicrophoneAGCEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MicrophoneAGCEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace MicrophoneAGCEnabled
namespace ImageRotation {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::ImageRotation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ImageRotation
namespace ImageFlipHorizontal {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::ImageFlipHorizontal::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ImageFlipHorizontal
namespace ImageFlipVertical {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::ImageFlipVertical::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ImageFlipVertical
namespace LocalVideoRecordingEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::LocalVideoRecordingEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LocalVideoRecordingEnabled
namespace LocalSnapshotRecordingEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::LocalSnapshotRecordingEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LocalSnapshotRecordingEnabled
namespace StatusLightEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::StatusLightEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace StatusLightEnabled
namespace StatusLightBrightness {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::StatusLightBrightness::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace StatusLightBrightness

} // namespace Attributes

namespace Commands {
namespace AudioStreamAllocate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CameraAvStreamManagement::Commands::AudioStreamAllocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace AudioStreamAllocate
namespace AudioStreamDeallocate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CameraAvStreamManagement::Commands::AudioStreamDeallocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace AudioStreamDeallocate
namespace VideoStreamAllocate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CameraAvStreamManagement::Commands::VideoStreamAllocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace VideoStreamAllocate
namespace VideoStreamModify {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CameraAvStreamManagement::Commands::VideoStreamModify::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace VideoStreamModify
namespace VideoStreamDeallocate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CameraAvStreamManagement::Commands::VideoStreamDeallocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace VideoStreamDeallocate
namespace SnapshotStreamAllocate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CameraAvStreamManagement::Commands::SnapshotStreamAllocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace SnapshotStreamAllocate
namespace SnapshotStreamModify {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CameraAvStreamManagement::Commands::SnapshotStreamModify::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace SnapshotStreamModify
namespace SnapshotStreamDeallocate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CameraAvStreamManagement::Commands::SnapshotStreamDeallocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace SnapshotStreamDeallocate
namespace SetStreamPriorities {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CameraAvStreamManagement::Commands::SetStreamPriorities::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SetStreamPriorities
namespace CaptureSnapshot {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CameraAvStreamManagement::Commands::CaptureSnapshot::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace CaptureSnapshot

} // namespace Commands
} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
