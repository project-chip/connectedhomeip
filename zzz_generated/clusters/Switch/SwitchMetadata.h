// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Switch (cluster code: 59/0x3B)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace Switch {

inline constexpr ClusterId kClusterId = 0x0000003B;

namespace Attributes {
namespace NumberOfPositions {
inline constexpr AttributeId Id = 0x00000000;
} // namespace NumberOfPositions
namespace CurrentPosition {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentPosition
namespace MultiPressMax {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MultiPressMax

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {
namespace SwitchLatched {
inline constexpr EventId Id = 0x00000000;
} // namespace SwitchLatched
namespace InitialPress {
inline constexpr EventId Id = 0x00000001;
} // namespace InitialPress
namespace LongPress {
inline constexpr EventId Id = 0x00000002;
} // namespace LongPress
namespace ShortRelease {
inline constexpr EventId Id = 0x00000003;
} // namespace ShortRelease
namespace LongRelease {
inline constexpr EventId Id = 0x00000004;
} // namespace LongRelease
namespace MultiPressOngoing {
inline constexpr EventId Id = 0x00000005;
} // namespace MultiPressOngoing
namespace MultiPressComplete {
inline constexpr EventId Id = 0x00000006;
} // namespace MultiPressComplete
} // namespace Events

} // namespace Switch
} // namespace clusters
} // namespace app
} // namespace chip
