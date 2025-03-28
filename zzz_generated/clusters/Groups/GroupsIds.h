// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Groups (cluster code: 4/0x4)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace Groups {

inline constexpr ClusterId kClusterId = 0x00000004;

namespace Attributes {
namespace NameSupport {
inline constexpr AttributeId Id = 0x00000000;
} // namespace NameSupport

// TODO: globals & reference globals?

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
} // namespace Commands

namespace Events {} // namespace Events

} // namespace Groups
} // namespace clusters
} // namespace app
} // namespace chip
