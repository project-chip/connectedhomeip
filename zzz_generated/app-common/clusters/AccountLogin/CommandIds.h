// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AccountLogin (cluster code: 1294/0x50E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AccountLogin {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 3;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace GetSetupPIN {
inline constexpr CommandId Id = 0x00000000;
} // namespace GetSetupPIN

namespace Login {
inline constexpr CommandId Id = 0x00000002;
} // namespace Login

namespace Logout {
inline constexpr CommandId Id = 0x00000003;
} // namespace Logout

namespace GetSetupPINResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace GetSetupPINResponse

} // namespace Commands
} // namespace AccountLogin
} // namespace Clusters
} // namespace app
} // namespace chip
