// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OvenMode (cluster code: 73/0x49)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OvenMode {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 1;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace ChangeToMode {
inline constexpr CommandId Id = 0x00000000;
} // namespace ChangeToMode

namespace ChangeToModeResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ChangeToModeResponse

} // namespace Commands
} // namespace OvenMode
} // namespace Clusters
} // namespace app
} // namespace chip
