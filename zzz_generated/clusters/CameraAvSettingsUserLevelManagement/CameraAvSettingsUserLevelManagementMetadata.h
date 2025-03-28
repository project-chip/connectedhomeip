// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster CameraAvSettingsUserLevelManagement (cluster code: 1362/0x552)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace CameraAvSettingsUserLevelManagement {

inline constexpr ClusterId kClusterId = 0x00000552;

namespace Attributes {
namespace MPTZPosition {
inline constexpr AttributeId Id = 0x00000000;
} // namespace MPTZPosition
namespace MaxPresets {
inline constexpr AttributeId Id = 0x00000001;
} // namespace MaxPresets
namespace MPTZPresets {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MPTZPresets
namespace DPTZRelativeMove {
inline constexpr AttributeId Id = 0x00000003;
} // namespace DPTZRelativeMove
namespace ZoomMax {
inline constexpr AttributeId Id = 0x00000004;
} // namespace ZoomMax
namespace TiltMin {
inline constexpr AttributeId Id = 0x00000005;
} // namespace TiltMin
namespace TiltMax {
inline constexpr AttributeId Id = 0x00000006;
} // namespace TiltMax
namespace PanMin {
inline constexpr AttributeId Id = 0x00000007;
} // namespace PanMin
namespace PanMax {
inline constexpr AttributeId Id = 0x00000008;
} // namespace PanMax

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace MPTZSetPosition {
inline constexpr CommandId Id = 0x00000000;
} // namespace MPTZSetPosition
namespace MPTZRelativeMove {
inline constexpr CommandId Id = 0x00000001;
} // namespace MPTZRelativeMove
namespace MPTZMoveToPreset {
inline constexpr CommandId Id = 0x00000002;
} // namespace MPTZMoveToPreset
namespace MPTZSavePreset {
inline constexpr CommandId Id = 0x00000003;
} // namespace MPTZSavePreset
namespace MPTZRemovePreset {
inline constexpr CommandId Id = 0x00000004;
} // namespace MPTZRemovePreset
namespace DPTZSetViewport {
inline constexpr CommandId Id = 0x00000005;
} // namespace DPTZSetViewport
namespace DPTZRelativeMove {
inline constexpr CommandId Id = 0x00000006;
} // namespace DPTZRelativeMove
} // namespace Commands

namespace Events {} // namespace Events

} // namespace CameraAvSettingsUserLevelManagement
} // namespace clusters
} // namespace app
} // namespace chip
