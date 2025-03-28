// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Timer (cluster code: 71/0x47)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace Timer {

inline constexpr ClusterId kClusterId = 0x00000047;

namespace Attributes {
namespace SetTime {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SetTime
namespace TimeRemaining {
inline constexpr AttributeId Id = 0x00000001;
} // namespace TimeRemaining
namespace TimerState {
inline constexpr AttributeId Id = 0x00000002;
} // namespace TimerState

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace SetTimer {
inline constexpr CommandId Id = 0x00000000;
} // namespace SetTimer
namespace ResetTimer {
inline constexpr CommandId Id = 0x00000001;
} // namespace ResetTimer
namespace AddTime {
inline constexpr CommandId Id = 0x00000002;
} // namespace AddTime
namespace ReduceTime {
inline constexpr CommandId Id = 0x00000003;
} // namespace ReduceTime
} // namespace Commands

namespace Events {} // namespace Events

} // namespace Timer
} // namespace clusters
} // namespace app
} // namespace chip
