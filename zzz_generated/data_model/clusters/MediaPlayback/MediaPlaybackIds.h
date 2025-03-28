// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster MediaPlayback (cluster code: 1286/0x506)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaPlayback {

inline constexpr ClusterId Id = 0x00000506;

namespace Attributes {
namespace CurrentState {
inline constexpr AttributeId Id = 0x00000000;
} // namespace CurrentState
namespace StartTime {
inline constexpr AttributeId Id = 0x00000001;
} // namespace StartTime
namespace Duration {
inline constexpr AttributeId Id = 0x00000002;
} // namespace Duration
namespace SampledPosition {
inline constexpr AttributeId Id = 0x00000003;
} // namespace SampledPosition
namespace PlaybackSpeed {
inline constexpr AttributeId Id = 0x00000004;
} // namespace PlaybackSpeed
namespace SeekRangeEnd {
inline constexpr AttributeId Id = 0x00000005;
} // namespace SeekRangeEnd
namespace SeekRangeStart {
inline constexpr AttributeId Id = 0x00000006;
} // namespace SeekRangeStart
namespace ActiveAudioTrack {
inline constexpr AttributeId Id = 0x00000007;
} // namespace ActiveAudioTrack
namespace AvailableAudioTracks {
inline constexpr AttributeId Id = 0x00000008;
} // namespace AvailableAudioTracks
namespace ActiveTextTrack {
inline constexpr AttributeId Id = 0x00000009;
} // namespace ActiveTextTrack
namespace AvailableTextTracks {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace AvailableTextTracks
namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList
namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList
namespace EventList {
inline constexpr AttributeId Id = Globals::Attributes::EventList::Id;
} // namespace EventList
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

namespace Commands {
namespace Play {
inline constexpr CommandId Id = 0x00000000;
} // namespace Play
namespace Pause {
inline constexpr CommandId Id = 0x00000001;
} // namespace Pause
namespace Stop {
inline constexpr CommandId Id = 0x00000002;
} // namespace Stop
namespace StartOver {
inline constexpr CommandId Id = 0x00000003;
} // namespace StartOver
namespace Previous {
inline constexpr CommandId Id = 0x00000004;
} // namespace Previous
namespace Next {
inline constexpr CommandId Id = 0x00000005;
} // namespace Next
namespace Rewind {
inline constexpr CommandId Id = 0x00000006;
} // namespace Rewind
namespace FastForward {
inline constexpr CommandId Id = 0x00000007;
} // namespace FastForward
namespace SkipForward {
inline constexpr CommandId Id = 0x00000008;
} // namespace SkipForward
namespace SkipBackward {
inline constexpr CommandId Id = 0x00000009;
} // namespace SkipBackward
namespace Seek {
inline constexpr CommandId Id = 0x0000000B;
} // namespace Seek
namespace ActivateAudioTrack {
inline constexpr CommandId Id = 0x0000000C;
} // namespace ActivateAudioTrack
namespace ActivateTextTrack {
inline constexpr CommandId Id = 0x0000000D;
} // namespace ActivateTextTrack
namespace DeactivateTextTrack {
inline constexpr CommandId Id = 0x0000000E;
} // namespace DeactivateTextTrack
namespace PlaybackResponse {
inline constexpr CommandId Id = 0x0000000A;
} // namespace PlaybackResponse
} // namespace Commands

namespace Events {
namespace StateChanged {
inline constexpr EventId Id = 0x00000000;
} // namespace StateChanged
} // namespace Events

} // namespace MediaPlayback
} // namespace Clusters
} // namespace app
} // namespace chip
