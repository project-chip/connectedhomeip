// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster TemperatureControl (cluster code: 86/0x56)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TemperatureControl {

inline constexpr ClusterId Id = 0x00000056;

namespace Attributes {
namespace TemperatureSetpoint {
inline constexpr AttributeId Id = 0x00000000;
} // namespace TemperatureSetpoint
namespace MinTemperature {
inline constexpr AttributeId Id = 0x00000001;
} // namespace MinTemperature
namespace MaxTemperature {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MaxTemperature
namespace Step {
inline constexpr AttributeId Id = 0x00000003;
} // namespace Step
namespace SelectedTemperatureLevel {
inline constexpr AttributeId Id = 0x00000004;
} // namespace SelectedTemperatureLevel
namespace SupportedTemperatureLevels {
inline constexpr AttributeId Id = 0x00000005;
} // namespace SupportedTemperatureLevels
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
namespace SetTemperature {
inline constexpr CommandId Id = 0x00000000;
} // namespace SetTemperature
} // namespace Commands

namespace Events {} // namespace Events

} // namespace TemperatureControl
} // namespace Clusters
} // namespace app
} // namespace chip
