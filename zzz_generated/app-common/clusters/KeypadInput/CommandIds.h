// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster KeypadInput (cluster code: 1289/0x509)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace KeypadInput {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 1;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace SendKey {
inline constexpr CommandId Id = 0x00000000;
} // namespace SendKey

namespace SendKeyResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace SendKeyResponse

} // namespace Commands
} // namespace KeypadInput
} // namespace Clusters
} // namespace app
} // namespace chip
