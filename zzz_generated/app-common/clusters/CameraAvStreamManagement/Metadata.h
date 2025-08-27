// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CameraAvStreamManagement (cluster code: 1361/0x551)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
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
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxConcurrentEncoders::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxConcurrentEncoders
namespace MaxEncodedPixelRate {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxEncodedPixelRate::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxEncodedPixelRate
namespace VideoSensorParams {
inline constexpr DataModel::AttributeEntry kMetadataEntry(VideoSensorParams::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace VideoSensorParams
namespace NightVisionUsesInfrared {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NightVisionUsesInfrared::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NightVisionUsesInfrared
namespace MinViewportResolution {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinViewportResolution::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinViewportResolution
namespace RateDistortionTradeOffPoints {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(RateDistortionTradeOffPoints::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace RateDistortionTradeOffPoints
namespace MaxContentBufferSize {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxContentBufferSize::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxContentBufferSize
namespace MicrophoneCapabilities {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MicrophoneCapabilities::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MicrophoneCapabilities
namespace SpeakerCapabilities {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SpeakerCapabilities::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SpeakerCapabilities
namespace TwoWayTalkSupport {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TwoWayTalkSupport::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TwoWayTalkSupport
namespace SnapshotCapabilities {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SnapshotCapabilities::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SnapshotCapabilities
namespace MaxNetworkBandwidth {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxNetworkBandwidth::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxNetworkBandwidth
namespace CurrentFrameRate {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentFrameRate::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentFrameRate
namespace HDRModeEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(HDRModeEnabled::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace HDRModeEnabled
namespace SupportedStreamUsages {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SupportedStreamUsages::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SupportedStreamUsages
namespace AllocatedVideoStreams {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AllocatedVideoStreams::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace AllocatedVideoStreams
namespace AllocatedAudioStreams {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AllocatedAudioStreams::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace AllocatedAudioStreams
namespace AllocatedSnapshotStreams {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AllocatedSnapshotStreams::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace AllocatedSnapshotStreams
namespace StreamUsagePriorities {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(StreamUsagePriorities::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace StreamUsagePriorities
namespace SoftRecordingPrivacyModeEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SoftRecordingPrivacyModeEnabled::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kOperate);
} // namespace SoftRecordingPrivacyModeEnabled
namespace SoftLivestreamPrivacyModeEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SoftLivestreamPrivacyModeEnabled::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kOperate);
} // namespace SoftLivestreamPrivacyModeEnabled
namespace HardPrivacyModeOn {
inline constexpr DataModel::AttributeEntry kMetadataEntry(HardPrivacyModeOn::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace HardPrivacyModeOn
namespace NightVision {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NightVision::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace NightVision
namespace NightVisionIllum {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NightVisionIllum::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace NightVisionIllum
namespace Viewport {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Viewport::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace Viewport
namespace SpeakerMuted {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SpeakerMuted::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace SpeakerMuted
namespace SpeakerVolumeLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SpeakerVolumeLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace SpeakerVolumeLevel
namespace SpeakerMaxLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SpeakerMaxLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, std::nullopt);
} // namespace SpeakerMaxLevel
namespace SpeakerMinLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SpeakerMinLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, std::nullopt);
} // namespace SpeakerMinLevel
namespace MicrophoneMuted {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MicrophoneMuted::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace MicrophoneMuted
namespace MicrophoneVolumeLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MicrophoneVolumeLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace MicrophoneVolumeLevel
namespace MicrophoneMaxLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MicrophoneMaxLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, std::nullopt);
} // namespace MicrophoneMaxLevel
namespace MicrophoneMinLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MicrophoneMinLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, std::nullopt);
} // namespace MicrophoneMinLevel
namespace MicrophoneAGCEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MicrophoneAGCEnabled::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace MicrophoneAGCEnabled
namespace ImageRotation {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ImageRotation::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace ImageRotation
namespace ImageFlipHorizontal {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ImageFlipHorizontal::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace ImageFlipHorizontal
namespace ImageFlipVertical {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ImageFlipVertical::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace ImageFlipVertical
namespace LocalVideoRecordingEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LocalVideoRecordingEnabled::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kManage,
                                                          Access::Privilege::kManage);
} // namespace LocalVideoRecordingEnabled
namespace LocalSnapshotRecordingEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LocalSnapshotRecordingEnabled::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kManage,
                                                          Access::Privilege::kManage);
} // namespace LocalSnapshotRecordingEnabled
namespace StatusLightEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StatusLightEnabled::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace StatusLightEnabled
namespace StatusLightBrightness {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StatusLightBrightness::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace StatusLightBrightness
constexpr std::array<DataModel::AttributeEntry, 4> kMandatoryMetadata = {
    MaxContentBufferSize::kMetadataEntry,
    MaxNetworkBandwidth::kMetadataEntry,
    SupportedStreamUsages::kMetadataEntry,
    StreamUsagePriorities::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace AudioStreamAllocate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AudioStreamAllocate::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace AudioStreamAllocate
namespace AudioStreamDeallocate {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(AudioStreamDeallocate::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace AudioStreamDeallocate
namespace VideoStreamAllocate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(VideoStreamAllocate::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace VideoStreamAllocate
namespace VideoStreamModify {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(VideoStreamModify::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace VideoStreamModify
namespace VideoStreamDeallocate {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(VideoStreamDeallocate::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace VideoStreamDeallocate
namespace SnapshotStreamAllocate {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SnapshotStreamAllocate::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace SnapshotStreamAllocate
namespace SnapshotStreamModify {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SnapshotStreamModify::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace SnapshotStreamModify
namespace SnapshotStreamDeallocate {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SnapshotStreamDeallocate::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace SnapshotStreamDeallocate
namespace SetStreamPriorities {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetStreamPriorities::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace SetStreamPriorities
namespace CaptureSnapshot {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(CaptureSnapshot::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace CaptureSnapshot

} // namespace Commands

namespace Events {} // namespace Events
} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
