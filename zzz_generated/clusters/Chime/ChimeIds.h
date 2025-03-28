// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Chime (cluster code: 1366/0x556)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace Chime {

inline constexpr ClusterId kClusterId = 0x00000556;

namespace Attributes {
namespace InstalledChimeSounds {
inline constexpr AttributeId Id = 0x00000000;
} // namespace InstalledChimeSounds
namespace SelectedChime {
inline constexpr AttributeId Id = 0x00000001;
} // namespace SelectedChime
namespace Enabled {
inline constexpr AttributeId Id = 0x00000002;
} // namespace Enabled

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace PlayChimeSound {
inline constexpr CommandId Id = 0x00000000;
} // namespace PlayChimeSound
} // namespace Commands

namespace Events {} // namespace Events

} // namespace Chime
} // namespace clusters
} // namespace app
} // namespace chip
