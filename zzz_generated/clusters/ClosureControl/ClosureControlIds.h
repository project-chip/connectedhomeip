// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ClosureControl (cluster code: 260/0x104)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

inline constexpr ClusterId Id = 0x00000104;

namespace Attributes {
namespace CountdownTime {
inline constexpr AttributeId Id = 0x00000000;
} // namespace CountdownTime
namespace MainState {
inline constexpr AttributeId Id = 0x00000001;
} // namespace MainState
namespace CurrentErrorList {
inline constexpr AttributeId Id = 0x00000002;
} // namespace CurrentErrorList
namespace OverallState {
inline constexpr AttributeId Id = 0x00000003;
} // namespace OverallState
namespace OverallTarget {
inline constexpr AttributeId Id = 0x00000004;
} // namespace OverallTarget
namespace RestingProcedure {
inline constexpr AttributeId Id = 0x00000005;
} // namespace RestingProcedure
namespace TriggerCondition {
inline constexpr AttributeId Id = 0x00000006;
} // namespace TriggerCondition
namespace TriggerPosition {
inline constexpr AttributeId Id = 0x00000007;
} // namespace TriggerPosition
namespace WaitingDelay {
inline constexpr AttributeId Id = 0x00000008;
} // namespace WaitingDelay
namespace KickoffTimer {
inline constexpr AttributeId Id = 0x00000009;
} // namespace KickoffTimer
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
namespace Stop {
inline constexpr CommandId Id = 0x00000000;
} // namespace Stop
namespace MoveTo {
inline constexpr CommandId Id = 0x00000001;
} // namespace MoveTo
namespace Calibrate {
inline constexpr CommandId Id = 0x00000002;
} // namespace Calibrate
namespace ConfigureFallback {
inline constexpr CommandId Id = 0x00000003;
} // namespace ConfigureFallback
namespace CancelFallback {
inline constexpr CommandId Id = 0x00000004;
} // namespace CancelFallback
} // namespace Commands

namespace Events {} // namespace Events

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
