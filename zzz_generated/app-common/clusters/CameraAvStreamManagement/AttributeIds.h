// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster CameraAvStreamManagement (cluster code: 1361/0x551)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvStreamManagement {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 46;

namespace MaxConcurrentEncoders {
inline constexpr AttributeId Id = 0x00000000;
} // namespace MaxConcurrentEncoders

namespace MaxEncodedPixelRate {
inline constexpr AttributeId Id = 0x00000001;
} // namespace MaxEncodedPixelRate

namespace VideoSensorParams {
inline constexpr AttributeId Id = 0x00000002;
} // namespace VideoSensorParams

namespace NightVisionUsesInfrared {
inline constexpr AttributeId Id = 0x00000003;
} // namespace NightVisionUsesInfrared

namespace MinViewportResolution {
inline constexpr AttributeId Id = 0x00000004;
} // namespace MinViewportResolution

namespace RateDistortionTradeOffPoints {
inline constexpr AttributeId Id = 0x00000005;
} // namespace RateDistortionTradeOffPoints

namespace MaxContentBufferSize {
inline constexpr AttributeId Id = 0x00000006;
} // namespace MaxContentBufferSize

namespace MicrophoneCapabilities {
inline constexpr AttributeId Id = 0x00000007;
} // namespace MicrophoneCapabilities

namespace SpeakerCapabilities {
inline constexpr AttributeId Id = 0x00000008;
} // namespace SpeakerCapabilities

namespace TwoWayTalkSupport {
inline constexpr AttributeId Id = 0x00000009;
} // namespace TwoWayTalkSupport

namespace SnapshotCapabilities {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace SnapshotCapabilities

namespace MaxNetworkBandwidth {
inline constexpr AttributeId Id = 0x0000000B;
} // namespace MaxNetworkBandwidth

namespace CurrentFrameRate {
inline constexpr AttributeId Id = 0x0000000C;
} // namespace CurrentFrameRate

namespace HDRModeEnabled {
inline constexpr AttributeId Id = 0x0000000D;
} // namespace HDRModeEnabled

namespace SupportedStreamUsages {
inline constexpr AttributeId Id = 0x0000000E;
} // namespace SupportedStreamUsages

namespace AllocatedVideoStreams {
inline constexpr AttributeId Id = 0x0000000F;
} // namespace AllocatedVideoStreams

namespace AllocatedAudioStreams {
inline constexpr AttributeId Id = 0x00000010;
} // namespace AllocatedAudioStreams

namespace AllocatedSnapshotStreams {
inline constexpr AttributeId Id = 0x00000011;
} // namespace AllocatedSnapshotStreams

namespace StreamUsagePriorities {
inline constexpr AttributeId Id = 0x00000012;
} // namespace StreamUsagePriorities

namespace SoftRecordingPrivacyModeEnabled {
inline constexpr AttributeId Id = 0x00000013;
} // namespace SoftRecordingPrivacyModeEnabled

namespace SoftLivestreamPrivacyModeEnabled {
inline constexpr AttributeId Id = 0x00000014;
} // namespace SoftLivestreamPrivacyModeEnabled

namespace HardPrivacyModeOn {
inline constexpr AttributeId Id = 0x00000015;
} // namespace HardPrivacyModeOn

namespace NightVision {
inline constexpr AttributeId Id = 0x00000016;
} // namespace NightVision

namespace NightVisionIllum {
inline constexpr AttributeId Id = 0x00000017;
} // namespace NightVisionIllum

namespace Viewport {
inline constexpr AttributeId Id = 0x00000018;
} // namespace Viewport

namespace SpeakerMuted {
inline constexpr AttributeId Id = 0x00000019;
} // namespace SpeakerMuted

namespace SpeakerVolumeLevel {
inline constexpr AttributeId Id = 0x0000001A;
} // namespace SpeakerVolumeLevel

namespace SpeakerMaxLevel {
inline constexpr AttributeId Id = 0x0000001B;
} // namespace SpeakerMaxLevel

namespace SpeakerMinLevel {
inline constexpr AttributeId Id = 0x0000001C;
} // namespace SpeakerMinLevel

namespace MicrophoneMuted {
inline constexpr AttributeId Id = 0x0000001D;
} // namespace MicrophoneMuted

namespace MicrophoneVolumeLevel {
inline constexpr AttributeId Id = 0x0000001E;
} // namespace MicrophoneVolumeLevel

namespace MicrophoneMaxLevel {
inline constexpr AttributeId Id = 0x0000001F;
} // namespace MicrophoneMaxLevel

namespace MicrophoneMinLevel {
inline constexpr AttributeId Id = 0x00000020;
} // namespace MicrophoneMinLevel

namespace MicrophoneAGCEnabled {
inline constexpr AttributeId Id = 0x00000021;
} // namespace MicrophoneAGCEnabled

namespace ImageRotation {
inline constexpr AttributeId Id = 0x00000022;
} // namespace ImageRotation

namespace ImageFlipHorizontal {
inline constexpr AttributeId Id = 0x00000023;
} // namespace ImageFlipHorizontal

namespace ImageFlipVertical {
inline constexpr AttributeId Id = 0x00000024;
} // namespace ImageFlipVertical

namespace LocalVideoRecordingEnabled {
inline constexpr AttributeId Id = 0x00000025;
} // namespace LocalVideoRecordingEnabled

namespace LocalSnapshotRecordingEnabled {
inline constexpr AttributeId Id = 0x00000026;
} // namespace LocalSnapshotRecordingEnabled

namespace StatusLightEnabled {
inline constexpr AttributeId Id = 0x00000027;
} // namespace StatusLightEnabled

namespace StatusLightBrightness {
inline constexpr AttributeId Id = 0x00000028;
} // namespace StatusLightBrightness

namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList

namespace AttributeList {
inline constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList

namespace FeatureMap {
inline constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap

namespace ClusterRevision {
inline constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

} // namespace Attributes
} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
