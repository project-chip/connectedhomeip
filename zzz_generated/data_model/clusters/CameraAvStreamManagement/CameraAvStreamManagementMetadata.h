// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CameraAvStreamManagement (cluster code: 1361/0x551)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/CameraAvStreamManagement/CameraAvStreamManagementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace CameraAvStreamManagement {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kMaxConcurrentVideoEncodersEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MaxConcurrentVideoEncoders::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxEncodedPixelRateEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MaxEncodedPixelRate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kVideoSensorParamsEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::VideoSensorParams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNightVisionCapableEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::NightVisionCapable::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMinViewportEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MinViewport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRateDistortionTradeOffPointsEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::RateDistortionTradeOffPoints::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxContentBufferSizeEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MaxContentBufferSize::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMicrophoneCapabilitiesEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MicrophoneCapabilities::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSpeakerCapabilitiesEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SpeakerCapabilities::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTwoWayTalkSupportEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::TwoWayTalkSupport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSupportedSnapshotParamsEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SupportedSnapshotParams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxNetworkBandwidthEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MaxNetworkBandwidth::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentFrameRateEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::CurrentFrameRate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kHDRModeEnabledEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::HDRModeEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSupportedStreamUsagesEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SupportedStreamUsages::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAllocatedVideoStreamsEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::AllocatedVideoStreams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAllocatedAudioStreamsEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::AllocatedAudioStreams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAllocatedSnapshotStreamsEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::AllocatedSnapshotStreams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRankedVideoStreamPrioritiesListEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::RankedVideoStreamPrioritiesList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSoftRecordingPrivacyModeEnabledEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SoftRecordingPrivacyModeEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kSoftLivestreamPrivacyModeEnabledEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SoftLivestreamPrivacyModeEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kHardPrivacyModeOnEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::HardPrivacyModeOn::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNightVisionEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::NightVision::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kNightVisionIllumEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::NightVisionIllum::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kViewportEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::Viewport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSpeakerMutedEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SpeakerMuted::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSpeakerVolumeLevelEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SpeakerVolumeLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSpeakerMaxLevelEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SpeakerMaxLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSpeakerMinLevelEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::SpeakerMinLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMicrophoneMutedEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MicrophoneMuted::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kMicrophoneVolumeLevelEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MicrophoneVolumeLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kMicrophoneMaxLevelEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MicrophoneMaxLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMicrophoneMinLevelEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MicrophoneMinLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMicrophoneAGCEnabledEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::MicrophoneAGCEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kImageRotationEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::ImageRotation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kImageFlipHorizontalEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::ImageFlipHorizontal::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kImageFlipVerticalEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::ImageFlipVertical::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kLocalVideoRecordingEnabledEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::LocalVideoRecordingEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kLocalSnapshotRecordingEnabledEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::LocalSnapshotRecordingEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kStatusLightEnabledEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::StatusLightEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kStatusLightBrightnessEntry = {
    .attributeId    = CameraAvStreamManagement::Attributes::StatusLightBrightness::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kAudioStreamAllocateEntry = {
    .commandId       = CameraAvStreamManagement::Commands::AudioStreamAllocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kAudioStreamDeallocateEntry = {
    .commandId       = CameraAvStreamManagement::Commands::AudioStreamDeallocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kVideoStreamAllocateEntry = {
    .commandId       = CameraAvStreamManagement::Commands::VideoStreamAllocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kVideoStreamModifyEntry = {
    .commandId       = CameraAvStreamManagement::Commands::VideoStreamModify::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kVideoStreamDeallocateEntry = {
    .commandId       = CameraAvStreamManagement::Commands::VideoStreamDeallocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kSnapshotStreamAllocateEntry = {
    .commandId       = CameraAvStreamManagement::Commands::SnapshotStreamAllocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kSnapshotStreamModifyEntry = {
    .commandId       = CameraAvStreamManagement::Commands::SnapshotStreamModify::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kSnapshotStreamDeallocateEntry = {
    .commandId       = CameraAvStreamManagement::Commands::SnapshotStreamDeallocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kSetStreamPrioritiesEntry = {
    .commandId       = CameraAvStreamManagement::Commands::SetStreamPriorities::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kCaptureSnapshotEntry = {
    .commandId       = CameraAvStreamManagement::Commands::CaptureSnapshot::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace CameraAvStreamManagement
} // namespace clusters
} // namespace app
} // namespace chip
