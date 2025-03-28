// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster TargetNavigator (cluster code: 1285/0x505)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace TargetNavigator {

inline constexpr ClusterId kClusterId = 0x00000505;

namespace Attributes {
namespace TargetList {
inline constexpr AttributeId Id = 0x00000000;
} // namespace TargetList
namespace CurrentTarget {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentTarget

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace NavigateTarget {
inline constexpr CommandId Id = 0x00000000;
} // namespace NavigateTarget
} // namespace Commands

namespace Events {
namespace TargetUpdated {
inline constexpr CommandId Id = 0x00000000;
} // namespace TargetUpdated
} // namespace Events
} // namespace TargetNavigator
} // namespace clusters
} // namespace app
} // namespace chip
