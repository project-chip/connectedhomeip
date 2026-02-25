// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster WindowCovering (cluster code: 258/0x102)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 5;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace UpOrOpen {
inline constexpr CommandId Id = 0x00000000;
} // namespace UpOrOpen

namespace DownOrClose {
inline constexpr CommandId Id = 0x00000001;
} // namespace DownOrClose

namespace StopMotion {
inline constexpr CommandId Id = 0x00000002;
} // namespace StopMotion

namespace GoToLiftPercentage {
inline constexpr CommandId Id = 0x00000005;
} // namespace GoToLiftPercentage

namespace GoToTiltPercentage {
inline constexpr CommandId Id = 0x00000008;
} // namespace GoToTiltPercentage

} // namespace Commands
} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip
