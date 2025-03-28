// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster FaultInjection (cluster code: 4294048774/0xFFF1FC06)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace FaultInjection {

inline constexpr ClusterId Id = 0xFFF1FC06;

namespace Attributes {
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
namespace FailAtFault {
inline constexpr CommandId Id = 0x00000000;
} // namespace FailAtFault
namespace FailRandomlyAtFault {
inline constexpr CommandId Id = 0x00000001;
} // namespace FailRandomlyAtFault
} // namespace Commands

namespace Events {} // namespace Events

} // namespace FaultInjection
} // namespace Clusters
} // namespace app
} // namespace chip
