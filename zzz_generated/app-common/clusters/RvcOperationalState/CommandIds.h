// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster RvcOperationalState (cluster code: 97/0x61)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace RvcOperationalState {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 3;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace Pause {
inline constexpr CommandId Id = 0x00000000;
} // namespace Pause

namespace Resume {
inline constexpr CommandId Id = 0x00000003;
} // namespace Resume

namespace GoHome {
inline constexpr CommandId Id = 0x00000080;
} // namespace GoHome

namespace OperationalCommandResponse {
inline constexpr CommandId Id = 0x00000004;
} // namespace OperationalCommandResponse

} // namespace Commands
} // namespace RvcOperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
