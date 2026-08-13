// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ProximityRanging (cluster code: 1075/0x433)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 2;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace StartRangingRequest {
inline constexpr CommandId Id = 0x00000000;
} // namespace StartRangingRequest

namespace StopRangingRequest {
inline constexpr CommandId Id = 0x00000002;
} // namespace StopRangingRequest

namespace StartRangingResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace StartRangingResponse

} // namespace Commands
} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
