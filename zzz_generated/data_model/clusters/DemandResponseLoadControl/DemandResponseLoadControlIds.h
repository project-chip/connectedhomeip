// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster DemandResponseLoadControl (cluster code: 150/0x96)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DemandResponseLoadControl {

inline constexpr ClusterId Id = 0x00000096;

namespace Attributes {
namespace LoadControlPrograms {
inline constexpr AttributeId Id = 0x00000000;
} // namespace LoadControlPrograms
namespace NumberOfLoadControlPrograms {
inline constexpr AttributeId Id = 0x00000001;
} // namespace NumberOfLoadControlPrograms
namespace Events {
inline constexpr AttributeId Id = 0x00000002;
} // namespace Events
namespace ActiveEvents {
inline constexpr AttributeId Id = 0x00000003;
} // namespace ActiveEvents
namespace NumberOfEventsPerProgram {
inline constexpr AttributeId Id = 0x00000004;
} // namespace NumberOfEventsPerProgram
namespace NumberOfTransitions {
inline constexpr AttributeId Id = 0x00000005;
} // namespace NumberOfTransitions
namespace DefaultRandomStart {
inline constexpr AttributeId Id = 0x00000006;
} // namespace DefaultRandomStart
namespace DefaultRandomDuration {
inline constexpr AttributeId Id = 0x00000007;
} // namespace DefaultRandomDuration
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
namespace RegisterLoadControlProgramRequest {
inline constexpr CommandId Id = 0x00000000;
} // namespace RegisterLoadControlProgramRequest
namespace UnregisterLoadControlProgramRequest {
inline constexpr CommandId Id = 0x00000001;
} // namespace UnregisterLoadControlProgramRequest
namespace AddLoadControlEventRequest {
inline constexpr CommandId Id = 0x00000002;
} // namespace AddLoadControlEventRequest
namespace RemoveLoadControlEventRequest {
inline constexpr CommandId Id = 0x00000003;
} // namespace RemoveLoadControlEventRequest
namespace ClearLoadControlEventsRequest {
inline constexpr CommandId Id = 0x00000004;
} // namespace ClearLoadControlEventsRequest
} // namespace Commands

namespace Events {
namespace LoadControlEventStatusChange {
inline constexpr EventId Id = 0x00000000;
} // namespace LoadControlEventStatusChange
} // namespace Events

} // namespace DemandResponseLoadControl
} // namespace Clusters
} // namespace app
} // namespace chip
