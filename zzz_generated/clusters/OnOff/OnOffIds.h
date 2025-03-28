// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OnOff (cluster code: 6/0x6)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace OnOff {

inline constexpr ClusterId kClusterId = 0x00000006;

namespace Attributes {
namespace OnOff {
inline constexpr AttributeId Id = 0x00000000;
} // namespace OnOff
namespace GlobalSceneControl {
inline constexpr AttributeId Id = 0x00004000;
} // namespace GlobalSceneControl
namespace OnTime {
inline constexpr AttributeId Id = 0x00004001;
} // namespace OnTime
namespace OffWaitTime {
inline constexpr AttributeId Id = 0x00004002;
} // namespace OffWaitTime
namespace StartUpOnOff {
inline constexpr AttributeId Id = 0x00004003;
} // namespace StartUpOnOff

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace Off {
inline constexpr CommandId Id = 0x00000000;
} // namespace Off
namespace On {
inline constexpr CommandId Id = 0x00000001;
} // namespace On
namespace Toggle {
inline constexpr CommandId Id = 0x00000002;
} // namespace Toggle
namespace OffWithEffect {
inline constexpr CommandId Id = 0x00000040;
} // namespace OffWithEffect
namespace OnWithRecallGlobalScene {
inline constexpr CommandId Id = 0x00000041;
} // namespace OnWithRecallGlobalScene
namespace OnWithTimedOff {
inline constexpr CommandId Id = 0x00000042;
} // namespace OnWithTimedOff
} // namespace Commands

namespace Events {} // namespace Events
} // namespace OnOff
} // namespace clusters
} // namespace app
} // namespace chip
