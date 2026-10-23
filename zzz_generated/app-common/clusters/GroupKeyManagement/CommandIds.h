// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster GroupKeyManagement (cluster code: 63/0x3F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GroupKeyManagement {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 4;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 2;

namespace KeySetWrite {
inline constexpr CommandId Id = 0x00000000;
} // namespace KeySetWrite

namespace KeySetRead {
inline constexpr CommandId Id = 0x00000001;
} // namespace KeySetRead

namespace KeySetRemove {
inline constexpr CommandId Id = 0x00000003;
} // namespace KeySetRemove

namespace KeySetReadAllIndices {
inline constexpr CommandId Id = 0x00000004;
} // namespace KeySetReadAllIndices

namespace KeySetReadResponse {
inline constexpr CommandId Id = 0x00000002;
} // namespace KeySetReadResponse

namespace KeySetReadAllIndicesResponse {
inline constexpr CommandId Id = 0x00000005;
} // namespace KeySetReadAllIndicesResponse

} // namespace Commands
} // namespace GroupKeyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
