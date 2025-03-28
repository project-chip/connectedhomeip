// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ApplicationLauncher (cluster code: 1292/0x50C)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace ApplicationLauncher {

inline constexpr ClusterId kClusterId = 0x0000050C;

namespace Attributes {
namespace CatalogList {
inline constexpr AttributeId Id = 0x00000000;
} // namespace CatalogList
namespace CurrentApp {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentApp

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace LaunchApp {
inline constexpr CommandId Id = 0x00000000;
} // namespace LaunchApp
namespace StopApp {
inline constexpr CommandId Id = 0x00000001;
} // namespace StopApp
namespace HideApp {
inline constexpr CommandId Id = 0x00000002;
} // namespace HideApp
} // namespace Commands

namespace Events {} // namespace Events
} // namespace ApplicationLauncher
} // namespace clusters
} // namespace app
} // namespace chip
