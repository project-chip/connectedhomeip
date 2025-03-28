// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Identify (cluster code: 3/0x3)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Identify {

inline constexpr ClusterId Id = 0x00000003;

namespace Attributes {
namespace IdentifyTime {
inline constexpr AttributeId Id = 0x00000000;
} // namespace IdentifyTime
namespace IdentifyType {
inline constexpr AttributeId Id = 0x00000001;
} // namespace IdentifyType
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
namespace Identify {
inline constexpr CommandId Id = 0x00000000;
} // namespace Identify
namespace TriggerEffect {
inline constexpr CommandId Id = 0x00000040;
} // namespace TriggerEffect
} // namespace Commands

namespace Events {} // namespace Events

} // namespace Identify
} // namespace Clusters
} // namespace app
} // namespace chip
