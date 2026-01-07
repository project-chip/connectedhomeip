// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ClosureDimension (cluster code: 261/0x105)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 2;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace SetTarget {
inline constexpr CommandId Id = 0x00000000;
} // namespace SetTarget

namespace Step {
inline constexpr CommandId Id = 0x00000001;
} // namespace Step

} // namespace Commands
} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
