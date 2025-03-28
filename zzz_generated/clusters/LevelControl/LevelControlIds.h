// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster LevelControl (cluster code: 8/0x8)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LevelControl {

inline constexpr ClusterId Id = 0x00000008;

namespace Attributes {
namespace CurrentLevel {
inline constexpr AttributeId Id = 0x00000000;
} // namespace CurrentLevel
namespace RemainingTime {
inline constexpr AttributeId Id = 0x00000001;
} // namespace RemainingTime
namespace MinLevel {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MinLevel
namespace MaxLevel {
inline constexpr AttributeId Id = 0x00000003;
} // namespace MaxLevel
namespace CurrentFrequency {
inline constexpr AttributeId Id = 0x00000004;
} // namespace CurrentFrequency
namespace MinFrequency {
inline constexpr AttributeId Id = 0x00000005;
} // namespace MinFrequency
namespace MaxFrequency {
inline constexpr AttributeId Id = 0x00000006;
} // namespace MaxFrequency
namespace Options {
inline constexpr AttributeId Id = 0x0000000F;
} // namespace Options
namespace OnOffTransitionTime {
inline constexpr AttributeId Id = 0x00000010;
} // namespace OnOffTransitionTime
namespace OnLevel {
inline constexpr AttributeId Id = 0x00000011;
} // namespace OnLevel
namespace OnTransitionTime {
inline constexpr AttributeId Id = 0x00000012;
} // namespace OnTransitionTime
namespace OffTransitionTime {
inline constexpr AttributeId Id = 0x00000013;
} // namespace OffTransitionTime
namespace DefaultMoveRate {
inline constexpr AttributeId Id = 0x00000014;
} // namespace DefaultMoveRate
namespace StartUpCurrentLevel {
inline constexpr AttributeId Id = 0x00004000;
} // namespace StartUpCurrentLevel
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
namespace MoveToLevel {
inline constexpr CommandId Id = 0x00000000;
} // namespace MoveToLevel
namespace Move {
inline constexpr CommandId Id = 0x00000001;
} // namespace Move
namespace Step {
inline constexpr CommandId Id = 0x00000002;
} // namespace Step
namespace Stop {
inline constexpr CommandId Id = 0x00000003;
} // namespace Stop
namespace MoveToLevelWithOnOff {
inline constexpr CommandId Id = 0x00000004;
} // namespace MoveToLevelWithOnOff
namespace MoveWithOnOff {
inline constexpr CommandId Id = 0x00000005;
} // namespace MoveWithOnOff
namespace StepWithOnOff {
inline constexpr CommandId Id = 0x00000006;
} // namespace StepWithOnOff
namespace StopWithOnOff {
inline constexpr CommandId Id = 0x00000007;
} // namespace StopWithOnOff
namespace MoveToClosestFrequency {
inline constexpr CommandId Id = 0x00000008;
} // namespace MoveToClosestFrequency
} // namespace Commands

namespace Events {} // namespace Events

} // namespace LevelControl
} // namespace Clusters
} // namespace app
} // namespace chip
