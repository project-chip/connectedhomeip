// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster TlsClientManagement (cluster code: 2050/0x802)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TlsClientManagement {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 3;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 2;

namespace ProvisionEndpoint {
inline constexpr CommandId Id = 0x00000000;
} // namespace ProvisionEndpoint

namespace FindEndpoint {
inline constexpr CommandId Id = 0x00000002;
} // namespace FindEndpoint

namespace RemoveEndpoint {
inline constexpr CommandId Id = 0x00000004;
} // namespace RemoveEndpoint

namespace ProvisionEndpointResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ProvisionEndpointResponse

namespace FindEndpointResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace FindEndpointResponse

} // namespace Commands
} // namespace TlsClientManagement
} // namespace Clusters
} // namespace app
} // namespace chip
