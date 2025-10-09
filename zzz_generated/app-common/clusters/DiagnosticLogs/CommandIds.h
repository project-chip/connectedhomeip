// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster DiagnosticLogs (cluster code: 50/0x32)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 1;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace RetrieveLogsRequest {
inline constexpr CommandId Id = 0x00000000;
} // namespace RetrieveLogsRequest

namespace RetrieveLogsResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace RetrieveLogsResponse

} // namespace Commands
} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
