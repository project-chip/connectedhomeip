// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster IcdManagement (cluster code: 70/0x46)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace IcdManagement {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 3;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 2;

namespace RegisterClient {
inline constexpr CommandId Id = 0x00000000;
} // namespace RegisterClient

namespace UnregisterClient {
inline constexpr CommandId Id = 0x00000002;
} // namespace UnregisterClient

namespace StayActiveRequest {
inline constexpr CommandId Id = 0x00000003;
} // namespace StayActiveRequest

namespace RegisterClientResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace RegisterClientResponse

namespace StayActiveResponse {
inline constexpr CommandId Id = 0x00000004;
} // namespace StayActiveResponse

} // namespace Commands
} // namespace IcdManagement
} // namespace Clusters
} // namespace app
} // namespace chip
