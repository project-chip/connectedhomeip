// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ThreadBorderRouterManagement (cluster code: 1106/0x452)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadBorderRouterManagement {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 4;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace GetActiveDatasetRequest {
inline constexpr CommandId Id = 0x00000000;
} // namespace GetActiveDatasetRequest

namespace GetPendingDatasetRequest {
inline constexpr CommandId Id = 0x00000001;
} // namespace GetPendingDatasetRequest

namespace SetActiveDatasetRequest {
inline constexpr CommandId Id = 0x00000003;
} // namespace SetActiveDatasetRequest

namespace SetPendingDatasetRequest {
inline constexpr CommandId Id = 0x00000004;
} // namespace SetPendingDatasetRequest

namespace DatasetResponse {
inline constexpr CommandId Id = 0x00000002;
} // namespace DatasetResponse

} // namespace Commands
} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
