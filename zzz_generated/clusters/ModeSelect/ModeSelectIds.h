// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ModeSelect (cluster code: 80/0x50)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace ModeSelect {

inline constexpr ClusterId kClusterId = 0x00000050;

namespace Attributes {
namespace Description {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Description
namespace StandardNamespace {
inline constexpr AttributeId Id = 0x00000001;
} // namespace StandardNamespace
namespace SupportedModes {
inline constexpr AttributeId Id = 0x00000002;
} // namespace SupportedModes
namespace CurrentMode {
inline constexpr AttributeId Id = 0x00000003;
} // namespace CurrentMode
namespace StartUpMode {
inline constexpr AttributeId Id = 0x00000004;
} // namespace StartUpMode
namespace OnMode {
inline constexpr AttributeId Id = 0x00000005;
} // namespace OnMode

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace ChangeToMode {
inline constexpr CommandId Id = 0x00000000;
} // namespace ChangeToMode
} // namespace Commands

namespace Events {} // namespace Events
} // namespace ModeSelect
} // namespace clusters
} // namespace app
} // namespace chip
