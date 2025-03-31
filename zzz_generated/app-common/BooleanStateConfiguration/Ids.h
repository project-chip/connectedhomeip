// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster BooleanStateConfiguration (cluster code: 128/0x80)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace BooleanStateConfiguration {

inline constexpr ClusterId Id = 0x00000080;

namespace Attributes {
namespace CurrentSensitivityLevel {
inline constexpr AttributeId Id = 0x00000000;
} // namespace CurrentSensitivityLevel
namespace SupportedSensitivityLevels {
inline constexpr AttributeId Id = 0x00000001;
} // namespace SupportedSensitivityLevels
namespace DefaultSensitivityLevel {
inline constexpr AttributeId Id = 0x00000002;
} // namespace DefaultSensitivityLevel
namespace AlarmsActive {
inline constexpr AttributeId Id = 0x00000003;
} // namespace AlarmsActive
namespace AlarmsSuppressed {
inline constexpr AttributeId Id = 0x00000004;
} // namespace AlarmsSuppressed
namespace AlarmsEnabled {
inline constexpr AttributeId Id = 0x00000005;
} // namespace AlarmsEnabled
namespace AlarmsSupported {
inline constexpr AttributeId Id = 0x00000006;
} // namespace AlarmsSupported
namespace SensorFault {
inline constexpr AttributeId Id = 0x00000007;
} // namespace SensorFault
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
namespace SuppressAlarm {
inline constexpr CommandId Id = 0x00000000;
} // namespace SuppressAlarm
namespace EnableDisableAlarm {
inline constexpr CommandId Id = 0x00000001;
} // namespace EnableDisableAlarm
} // namespace Commands

namespace Events {
namespace AlarmsStateChanged {
inline constexpr EventId Id = 0x00000000;
} // namespace AlarmsStateChanged
namespace SensorFault {
inline constexpr EventId Id = 0x00000001;
} // namespace SensorFault
} // namespace Events

} // namespace BooleanStateConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip
