// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Groups (cluster code: 4/0x4)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Groups {

inline constexpr ClusterId Id = 0x00000004;

namespace Attributes {
namespace NameSupport {
inline constexpr AttributeId Id = 0x00000000;
} // namespace NameSupport
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
namespace AddGroup {
inline constexpr CommandId Id = 0x00000000;
} // namespace AddGroup
namespace ViewGroup {
inline constexpr CommandId Id = 0x00000001;
} // namespace ViewGroup
namespace GetGroupMembership {
inline constexpr CommandId Id = 0x00000002;
} // namespace GetGroupMembership
namespace RemoveGroup {
inline constexpr CommandId Id = 0x00000003;
} // namespace RemoveGroup
namespace RemoveAllGroups {
inline constexpr CommandId Id = 0x00000004;
} // namespace RemoveAllGroups
namespace AddGroupIfIdentifying {
inline constexpr CommandId Id = 0x00000005;
} // namespace AddGroupIfIdentifying
namespace AddGroupResponse {
inline constexpr CommandId Id = 0x00000000;
} // namespace AddGroupResponse
namespace ViewGroupResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ViewGroupResponse
namespace GetGroupMembershipResponse {
inline constexpr CommandId Id = 0x00000002;
} // namespace GetGroupMembershipResponse
namespace RemoveGroupResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace RemoveGroupResponse
} // namespace Commands

namespace Events {} // namespace Events

} // namespace Groups
} // namespace Clusters
} // namespace app
} // namespace chip
