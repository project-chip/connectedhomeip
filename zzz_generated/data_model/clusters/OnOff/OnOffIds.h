// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OnOff (cluster code: 6/0x6)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OnOff {

inline constexpr ClusterId Id = 0x00000006;

namespace Attributes {
namespace OnOff {
inline constexpr AttributeId Id = 0x00000000;
} // namespace OnOff
namespace GlobalSceneControl {
inline constexpr AttributeId Id = 0x00004000;
} // namespace GlobalSceneControl
namespace OnTime {
inline constexpr AttributeId Id = 0x00004001;
} // namespace OnTime
namespace OffWaitTime {
inline constexpr AttributeId Id = 0x00004002;
} // namespace OffWaitTime
namespace StartUpOnOff {
inline constexpr AttributeId Id = 0x00004003;
} // namespace StartUpOnOff
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
namespace Off {
inline constexpr CommandId Id = 0x00000000;
} // namespace Off
namespace On {
inline constexpr CommandId Id = 0x00000001;
} // namespace On
namespace Toggle {
inline constexpr CommandId Id = 0x00000002;
} // namespace Toggle
namespace OffWithEffect {
inline constexpr CommandId Id = 0x00000040;
} // namespace OffWithEffect
namespace OnWithRecallGlobalScene {
inline constexpr CommandId Id = 0x00000041;
} // namespace OnWithRecallGlobalScene
namespace OnWithTimedOff {
inline constexpr CommandId Id = 0x00000042;
} // namespace OnWithTimedOff
} // namespace Commands

namespace Events {} // namespace Events

} // namespace OnOff
} // namespace Clusters
} // namespace app
} // namespace chip
