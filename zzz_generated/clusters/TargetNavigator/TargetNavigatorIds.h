// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster TargetNavigator (cluster code: 1285/0x505)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TargetNavigator {

inline constexpr ClusterId Id = 0x00000505;

namespace Attributes {
namespace TargetList {
inline constexpr AttributeId Id = 0x00000000;
} // namespace TargetList
namespace CurrentTarget {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentTarget
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
namespace NavigateTarget {
inline constexpr CommandId Id = 0x00000000;
} // namespace NavigateTarget
namespace NavigateTargetResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace NavigateTargetResponse
} // namespace Commands

namespace Events {
namespace TargetUpdated {
inline constexpr EventId Id = 0x00000000;
} // namespace TargetUpdated
} // namespace Events

} // namespace TargetNavigator
} // namespace Clusters
} // namespace app
} // namespace chip
