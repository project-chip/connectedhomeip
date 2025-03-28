// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Timer (cluster code: 71/0x47)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Timer {

inline constexpr ClusterId Id = 0x00000047;

namespace Attributes {
namespace SetTime {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SetTime
namespace TimeRemaining {
inline constexpr AttributeId Id = 0x00000001;
} // namespace TimeRemaining
namespace TimerState {
inline constexpr AttributeId Id = 0x00000002;
} // namespace TimerState
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
namespace SetTimer {
inline constexpr CommandId Id = 0x00000000;
} // namespace SetTimer
namespace ResetTimer {
inline constexpr CommandId Id = 0x00000001;
} // namespace ResetTimer
namespace AddTime {
inline constexpr CommandId Id = 0x00000002;
} // namespace AddTime
namespace ReduceTime {
inline constexpr CommandId Id = 0x00000003;
} // namespace ReduceTime
} // namespace Commands

namespace Events {} // namespace Events

} // namespace Timer
} // namespace Clusters
} // namespace app
} // namespace chip
