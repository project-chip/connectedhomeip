// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CameraAvStreamManagement (cluster code: 1361/0x551)
// based on src/controller/data_model/controller-clusters.matter

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
    .attributeId    = Attributes::MaxConcurrentVideoEncoders::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxEncodedPixelRateEntry = {
    .attributeId    = Attributes::MaxEncodedPixelRate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kVideoSensorParamsEntry = {
    .attributeId    = Attributes::VideoSensorParams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNightVisionCapableEntry = {
    .attributeId    = Attributes::NightVisionCapable::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMinViewportEntry = {
    .attributeId    = Attributes::MinViewport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRateDistortionTradeOffPointsEntry = {
    .attributeId    = Attributes::RateDistortionTradeOffPoints::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxContentBufferSizeEntry = {
    .attributeId    = Attributes::MaxContentBufferSize::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMicrophoneCapabilitiesEntry = {
    .attributeId    = Attributes::MicrophoneCapabilities::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSpeakerCapabilitiesEntry = {
    .attributeId    = Attributes::SpeakerCapabilities::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTwoWayTalkSupportEntry = {
    .attributeId    = Attributes::TwoWayTalkSupport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSupportedSnapshotParamsEntry = {
    .attributeId    = Attributes::SupportedSnapshotParams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxNetworkBandwidthEntry = {
    .attributeId    = Attributes::MaxNetworkBandwidth::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentFrameRateEntry = {
    .attributeId    = Attributes::CurrentFrameRate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kHDRModeEnabledEntry = {
    .attributeId    = Attributes::HDRModeEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSupportedStreamUsagesEntry = {
    .attributeId    = Attributes::SupportedStreamUsages::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAllocatedVideoStreamsEntry = {
    .attributeId    = Attributes::AllocatedVideoStreams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAllocatedAudioStreamsEntry = {
    .attributeId    = Attributes::AllocatedAudioStreams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAllocatedSnapshotStreamsEntry = {
    .attributeId    = Attributes::AllocatedSnapshotStreams::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kRankedVideoStreamPrioritiesListEntry = {
    .attributeId    = Attributes::RankedVideoStreamPrioritiesList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSoftRecordingPrivacyModeEnabledEntry = {
    .attributeId    = Attributes::SoftRecordingPrivacyModeEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kSoftLivestreamPrivacyModeEnabledEntry = {
    .attributeId    = Attributes::SoftLivestreamPrivacyModeEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kHardPrivacyModeOnEntry = {
    .attributeId    = Attributes::HardPrivacyModeOn::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNightVisionEntry = {
    .attributeId    = Attributes::NightVision::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kNightVisionIllumEntry = {
    .attributeId    = Attributes::NightVisionIllum::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kViewportEntry = {
    .attributeId    = Attributes::Viewport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSpeakerMutedEntry = {
    .attributeId    = Attributes::SpeakerMuted::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSpeakerVolumeLevelEntry = {
    .attributeId    = Attributes::SpeakerVolumeLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSpeakerMaxLevelEntry = {
    .attributeId    = Attributes::SpeakerMaxLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSpeakerMinLevelEntry = {
    .attributeId    = Attributes::SpeakerMinLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMicrophoneMutedEntry = {
    .attributeId    = Attributes::MicrophoneMuted::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kMicrophoneVolumeLevelEntry = {
    .attributeId    = Attributes::MicrophoneVolumeLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kMicrophoneMaxLevelEntry = {
    .attributeId    = Attributes::MicrophoneMaxLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMicrophoneMinLevelEntry = {
    .attributeId    = Attributes::MicrophoneMinLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMicrophoneAGCEnabledEntry = {
    .attributeId    = Attributes::MicrophoneAGCEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kImageRotationEntry = {
    .attributeId    = Attributes::ImageRotation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kImageFlipHorizontalEntry = {
    .attributeId    = Attributes::ImageFlipHorizontal::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kImageFlipVerticalEntry = {
    .attributeId    = Attributes::ImageFlipVertical::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kLocalVideoRecordingEnabledEntry = {
    .attributeId    = Attributes::LocalVideoRecordingEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kLocalSnapshotRecordingEnabledEntry = {
    .attributeId    = Attributes::LocalSnapshotRecordingEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kStatusLightEnabledEntry = {
    .attributeId    = Attributes::StatusLightEnabled::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kStatusLightBrightnessEntry = {
    .attributeId    = Attributes::StatusLightBrightness::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kManage,
    .writePrivilege = Access::Privilege::kManage,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kAudioStreamAllocateEntry = {
    .commandId       = Commands::AudioStreamAllocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kAudioStreamDeallocateEntry = {
    .commandId       = Commands::AudioStreamDeallocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kVideoStreamAllocateEntry = {
    .commandId       = Commands::VideoStreamAllocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kVideoStreamModifyEntry = {
    .commandId       = Commands::VideoStreamModify::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kVideoStreamDeallocateEntry = {
    .commandId       = Commands::VideoStreamDeallocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kSnapshotStreamAllocateEntry = {
    .commandId       = Commands::SnapshotStreamAllocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kSnapshotStreamModifyEntry = {
    .commandId       = Commands::SnapshotStreamModify::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kSnapshotStreamDeallocateEntry = {
    .commandId       = Commands::SnapshotStreamDeallocate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kSetStreamPrioritiesEntry = {
    .commandId       = Commands::SetStreamPriorities::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kCaptureSnapshotEntry = {
    .commandId       = Commands::CaptureSnapshot::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace CameraAvStreamManagement
} // namespace clusters
} // namespace app
} // namespace chip
