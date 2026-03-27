// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ClosureControl (cluster code: 260/0x104)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 3;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace Stop {
inline constexpr CommandId Id = 0x00000000;
} // namespace Stop

namespace MoveTo {
inline constexpr CommandId Id = 0x00000001;
} // namespace MoveTo

namespace Calibrate {
inline constexpr CommandId Id = 0x00000002;
} // namespace Calibrate

} // namespace Commands
} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
