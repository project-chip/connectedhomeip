// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Channel (cluster code: 1284/0x504)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Channel {

inline constexpr ClusterId Id = 0x00000504;

namespace Attributes {
namespace ChannelList {
inline constexpr AttributeId Id = 0x00000000;
} // namespace ChannelList
namespace Lineup {
inline constexpr AttributeId Id = 0x00000001;
} // namespace Lineup
namespace CurrentChannel {
inline constexpr AttributeId Id = 0x00000002;
} // namespace CurrentChannel
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
namespace ChangeChannel {
inline constexpr CommandId Id = 0x00000000;
} // namespace ChangeChannel
namespace ChangeChannelByNumber {
inline constexpr CommandId Id = 0x00000002;
} // namespace ChangeChannelByNumber
namespace SkipChannel {
inline constexpr CommandId Id = 0x00000003;
} // namespace SkipChannel
namespace GetProgramGuide {
inline constexpr CommandId Id = 0x00000004;
} // namespace GetProgramGuide
namespace RecordProgram {
inline constexpr CommandId Id = 0x00000006;
} // namespace RecordProgram
namespace CancelRecordProgram {
inline constexpr CommandId Id = 0x00000007;
} // namespace CancelRecordProgram
namespace ChangeChannelResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ChangeChannelResponse
namespace ProgramGuideResponse {
inline constexpr CommandId Id = 0x00000005;
} // namespace ProgramGuideResponse
} // namespace Commands

namespace Events {} // namespace Events

} // namespace Channel
} // namespace Clusters
} // namespace app
} // namespace chip
