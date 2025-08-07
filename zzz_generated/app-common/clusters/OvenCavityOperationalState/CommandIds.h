// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OvenCavityOperationalState (cluster code: 72/0x48)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OvenCavityOperationalState {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 2;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace Stop {
inline constexpr CommandId Id = 0x00000001;
} // namespace Stop

namespace Start {
inline constexpr CommandId Id = 0x00000002;
} // namespace Start

namespace OperationalCommandResponse {
inline constexpr CommandId Id = 0x00000004;
} // namespace OperationalCommandResponse

} // namespace Commands
} // namespace OvenCavityOperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
