// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster DishwasherAlarm (cluster code: 93/0x5D)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace DishwasherAlarm {

inline constexpr ClusterId kClusterId = 0x0000005D;

namespace Attributes {
namespace Mask {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Mask
namespace Latch {
inline constexpr AttributeId Id = 0x00000001;
} // namespace Latch
namespace State {
inline constexpr AttributeId Id = 0x00000002;
} // namespace State
namespace Supported {
inline constexpr AttributeId Id = 0x00000003;
} // namespace Supported

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace Reset {
inline constexpr CommandId Id = 0x00000000;
} // namespace Reset
namespace ModifyEnabledAlarms {
inline constexpr CommandId Id = 0x00000001;
} // namespace ModifyEnabledAlarms
} // namespace Commands

namespace Events {
namespace Notify {
inline constexpr EventId Id = 0x00000000;
} // namespace Notify
} // namespace Events
} // namespace DishwasherAlarm
} // namespace clusters
} // namespace app
} // namespace chip
