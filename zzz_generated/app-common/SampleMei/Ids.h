// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster SampleMei (cluster code: 4294048800/0xFFF1FC20)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SampleMei {

inline constexpr ClusterId Id = 0xFFF1FC20;

namespace Attributes {
namespace FlipFlop {
inline constexpr AttributeId Id = 0x00000000;
} // namespace FlipFlop
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
namespace Ping {
inline constexpr CommandId Id = 0x00000000;
} // namespace Ping
namespace AddArguments {
inline constexpr CommandId Id = 0x00000002;
} // namespace AddArguments
namespace AddArgumentsResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace AddArgumentsResponse
} // namespace Commands

namespace Events {
namespace PingCountEvent {
inline constexpr EventId Id = 0x00000000;
} // namespace PingCountEvent
} // namespace Events

} // namespace SampleMei
} // namespace Clusters
} // namespace app
} // namespace chip
