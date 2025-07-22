// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ContentLauncher (cluster code: 1290/0x50A)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ContentLauncher {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 2;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace LaunchContent {
inline constexpr CommandId Id = 0x00000000;
} // namespace LaunchContent

namespace LaunchURL {
inline constexpr CommandId Id = 0x00000001;
} // namespace LaunchURL

namespace LauncherResponse {
inline constexpr CommandId Id = 0x00000002;
} // namespace LauncherResponse

} // namespace Commands
} // namespace ContentLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
