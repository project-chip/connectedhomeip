// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AccessControl (cluster code: 31/0x1F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AccessControl {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 1;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace ReviewFabricRestrictions {
inline constexpr CommandId Id = 0x00000000;
} // namespace ReviewFabricRestrictions

namespace ReviewFabricRestrictionsResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ReviewFabricRestrictionsResponse

} // namespace Commands
} // namespace AccessControl
} // namespace Clusters
} // namespace app
} // namespace chip
