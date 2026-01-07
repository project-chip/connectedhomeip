// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster FaultInjection (cluster code: 4294048774/0xFFF1FC06)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace FaultInjection {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 2;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace FailAtFault {
inline constexpr CommandId Id = 0x00000000;
} // namespace FailAtFault

namespace FailRandomlyAtFault {
inline constexpr CommandId Id = 0x00000001;
} // namespace FailRandomlyAtFault

} // namespace Commands
} // namespace FaultInjection
} // namespace Clusters
} // namespace app
} // namespace chip
