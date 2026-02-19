// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Groups (cluster code: 4/0x4)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Groups {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 6;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 4;

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
} // namespace Groups
} // namespace Clusters
} // namespace app
} // namespace chip
