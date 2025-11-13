// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Timer (cluster code: 71/0x47)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Timer {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 4;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace SetTimer {
inline constexpr CommandId Id = 0x00000000;
} // namespace SetTimer

namespace ResetTimer {
inline constexpr CommandId Id = 0x00000001;
} // namespace ResetTimer

namespace AddTime {
inline constexpr CommandId Id = 0x00000002;
} // namespace AddTime

namespace ReduceTime {
inline constexpr CommandId Id = 0x00000003;
} // namespace ReduceTime

} // namespace Commands
} // namespace Timer
} // namespace Clusters
} // namespace app
} // namespace chip
