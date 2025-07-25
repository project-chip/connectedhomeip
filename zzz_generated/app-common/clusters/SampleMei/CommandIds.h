// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster SampleMei (cluster code: 4294048800/0xFFF1FC20)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SampleMei {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 2;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace Ping {
inline constexpr CommandId Id = 0x00000000;
} // namespace Ping

namespace AddArguments {
inline constexpr CommandId Id = 0x00000002;
} // namespace AddArguments

namespace AddArgumentsResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace AddArgumentsResponse

} // namespace Commands
} // namespace SampleMei
} // namespace Clusters
} // namespace app
} // namespace chip
