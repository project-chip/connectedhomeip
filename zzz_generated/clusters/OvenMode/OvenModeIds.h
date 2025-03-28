// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OvenMode (cluster code: 73/0x49)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace OvenMode {

inline constexpr ClusterId kClusterId = 0x00000049;

namespace Attributes {
namespace SupportedModes {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SupportedModes
namespace CurrentMode {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentMode
namespace StartUpMode {
inline constexpr AttributeId Id = 0x00000002;
} // namespace StartUpMode
namespace OnMode {
inline constexpr AttributeId Id = 0x00000003;
} // namespace OnMode

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace ChangeToMode {
inline constexpr CommandId Id = 0x00000000;
} // namespace ChangeToMode
} // namespace Commands

namespace Events {} // namespace Events

} // namespace OvenMode
} // namespace clusters
} // namespace app
} // namespace chip
