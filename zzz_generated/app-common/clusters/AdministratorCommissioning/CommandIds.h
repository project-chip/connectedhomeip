// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AdministratorCommissioning (cluster code: 60/0x3C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AdministratorCommissioning {
namespace Commands {

inline constexpr uint32_t kAcceptedCommandsCount  = 3; // Total number of commands without a response supported by the cluster
inline constexpr uint32_t kGeneratedCommandsCount = 0; // Total number of commands with a response supported by the cluster

namespace OpenCommissioningWindow {
inline constexpr CommandId Id = 0x00000000;
} // namespace OpenCommissioningWindow

namespace OpenBasicCommissioningWindow {
inline constexpr CommandId Id = 0x00000001;
} // namespace OpenBasicCommissioningWindow

namespace RevokeCommissioning {
inline constexpr CommandId Id = 0x00000002;
} // namespace RevokeCommissioning

} // namespace Commands
} // namespace AdministratorCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
