// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster NetworkIdentityManagement (cluster code: 1104/0x450)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkIdentityManagement {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 5;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 3;

namespace AddClient {
inline constexpr CommandId Id = 0x00000000;
} // namespace AddClient

namespace RemoveClient {
inline constexpr CommandId Id = 0x00000002;
} // namespace RemoveClient

namespace QueryIdentity {
inline constexpr CommandId Id = 0x00000003;
} // namespace QueryIdentity

namespace ImportAdminSecret {
inline constexpr CommandId Id = 0x00000040;
} // namespace ImportAdminSecret

namespace ExportAdminSecret {
inline constexpr CommandId Id = 0x00000041;
} // namespace ExportAdminSecret

namespace AddClientResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace AddClientResponse

namespace QueryIdentityResponse {
inline constexpr CommandId Id = 0x00000004;
} // namespace QueryIdentityResponse

namespace ExportAdminSecretResponse {
inline constexpr CommandId Id = 0x00000042;
} // namespace ExportAdminSecretResponse

} // namespace Commands
} // namespace NetworkIdentityManagement
} // namespace Clusters
} // namespace app
} // namespace chip
