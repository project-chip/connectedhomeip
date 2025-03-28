// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster FanControl (cluster code: 514/0x202)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace FanControl {

inline constexpr ClusterId Id = 0x00000202;

namespace Attributes {
namespace FanMode {
inline constexpr AttributeId Id = 0x00000000;
} // namespace FanMode
namespace FanModeSequence {
inline constexpr AttributeId Id = 0x00000001;
} // namespace FanModeSequence
namespace PercentSetting {
inline constexpr AttributeId Id = 0x00000002;
} // namespace PercentSetting
namespace PercentCurrent {
inline constexpr AttributeId Id = 0x00000003;
} // namespace PercentCurrent
namespace SpeedMax {
inline constexpr AttributeId Id = 0x00000004;
} // namespace SpeedMax
namespace SpeedSetting {
inline constexpr AttributeId Id = 0x00000005;
} // namespace SpeedSetting
namespace SpeedCurrent {
inline constexpr AttributeId Id = 0x00000006;
} // namespace SpeedCurrent
namespace RockSupport {
inline constexpr AttributeId Id = 0x00000007;
} // namespace RockSupport
namespace RockSetting {
inline constexpr AttributeId Id = 0x00000008;
} // namespace RockSetting
namespace WindSupport {
inline constexpr AttributeId Id = 0x00000009;
} // namespace WindSupport
namespace WindSetting {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace WindSetting
namespace AirflowDirection {
inline constexpr AttributeId Id = 0x0000000B;
} // namespace AirflowDirection
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
namespace Step {
inline constexpr CommandId Id = 0x00000000;
} // namespace Step
} // namespace Commands

namespace Events {} // namespace Events

} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace chip
