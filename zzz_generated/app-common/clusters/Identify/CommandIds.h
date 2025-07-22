// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Identify (cluster code: 3/0x3)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Identify {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 2;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace Identify {
inline constexpr CommandId Id = 0x00000000;
} // namespace Identify

namespace TriggerEffect {
inline constexpr CommandId Id = 0x00000040;
} // namespace TriggerEffect

} // namespace Commands
} // namespace Identify
} // namespace Clusters
} // namespace app
} // namespace chip
