// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ServiceArea (cluster code: 336/0x150)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 2;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 2;

namespace SelectAreas {
inline constexpr CommandId Id = 0x00000000;
} // namespace SelectAreas

namespace SkipArea {
inline constexpr CommandId Id = 0x00000002;
} // namespace SkipArea

namespace SelectAreasResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace SelectAreasResponse

namespace SkipAreaResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace SkipAreaResponse

} // namespace Commands
} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
