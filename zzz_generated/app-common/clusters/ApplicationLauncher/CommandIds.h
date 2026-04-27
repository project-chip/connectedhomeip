// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ApplicationLauncher (cluster code: 1292/0x50C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationLauncher {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 3;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace LaunchApp {
inline constexpr CommandId Id = 0x00000000;
} // namespace LaunchApp

namespace StopApp {
inline constexpr CommandId Id = 0x00000001;
} // namespace StopApp

namespace HideApp {
inline constexpr CommandId Id = 0x00000002;
} // namespace HideApp

namespace LauncherResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace LauncherResponse

} // namespace Commands
} // namespace ApplicationLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
