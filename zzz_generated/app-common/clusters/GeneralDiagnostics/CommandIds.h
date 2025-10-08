// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster GeneralDiagnostics (cluster code: 51/0x33)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralDiagnostics {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 3;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 2;

namespace TestEventTrigger {
inline constexpr CommandId Id = 0x00000000;
} // namespace TestEventTrigger

namespace TimeSnapshot {
inline constexpr CommandId Id = 0x00000001;
} // namespace TimeSnapshot

namespace PayloadTestRequest {
inline constexpr CommandId Id = 0x00000003;
} // namespace PayloadTestRequest

namespace TimeSnapshotResponse {
inline constexpr CommandId Id = 0x00000002;
} // namespace TimeSnapshotResponse

namespace PayloadTestResponse {
inline constexpr CommandId Id = 0x00000004;
} // namespace PayloadTestResponse

} // namespace Commands
} // namespace GeneralDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
