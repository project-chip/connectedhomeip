// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ThreadNetworkDirectory (cluster code: 1107/0x453)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadNetworkDirectory {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 3;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace AddNetwork {
inline constexpr CommandId Id = 0x00000000;
} // namespace AddNetwork

namespace RemoveNetwork {
inline constexpr CommandId Id = 0x00000001;
} // namespace RemoveNetwork

namespace GetOperationalDataset {
inline constexpr CommandId Id = 0x00000002;
} // namespace GetOperationalDataset

namespace OperationalDatasetResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace OperationalDatasetResponse

} // namespace Commands
} // namespace ThreadNetworkDirectory
} // namespace Clusters
} // namespace app
} // namespace chip
