// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OperationalState (cluster code: 96/0x60)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 4;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace Pause {
inline constexpr CommandId Id = 0x00000000;
} // namespace Pause

namespace Stop {
inline constexpr CommandId Id = 0x00000001;
} // namespace Stop

namespace Start {
inline constexpr CommandId Id = 0x00000002;
} // namespace Start

namespace Resume {
inline constexpr CommandId Id = 0x00000003;
} // namespace Resume

namespace OperationalCommandResponse {
inline constexpr CommandId Id = 0x00000004;
} // namespace OperationalCommandResponse

} // namespace Commands
} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
