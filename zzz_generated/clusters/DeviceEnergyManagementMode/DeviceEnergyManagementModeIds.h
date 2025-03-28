// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster DeviceEnergyManagementMode (cluster code: 159/0x9F)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace DeviceEnergyManagementMode {

inline constexpr ClusterId kClusterId = 0x0000009F;

namespace Attributes {
namespace SupportedModes {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SupportedModes
namespace CurrentMode {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentMode

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace ChangeToMode {
inline constexpr CommandId Id = 0x00000000;
} // namespace ChangeToMode
} // namespace Commands

namespace Events {} // namespace Events

} // namespace DeviceEnergyManagementMode
} // namespace clusters
} // namespace app
} // namespace chip
