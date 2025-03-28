// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster EnergyEvseMode (cluster code: 157/0x9D)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace EnergyEvseMode {

inline constexpr ClusterId kClusterId = 0x0000009D;

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

} // namespace EnergyEvseMode
} // namespace clusters
} // namespace app
} // namespace chip
