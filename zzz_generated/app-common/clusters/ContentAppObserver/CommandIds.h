// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ContentAppObserver (cluster code: 1296/0x510)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ContentAppObserver {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 1;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace ContentAppMessage {
inline constexpr CommandId Id = 0x00000000;
} // namespace ContentAppMessage

namespace ContentAppMessageResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ContentAppMessageResponse

} // namespace Commands
} // namespace ContentAppObserver
} // namespace Clusters
} // namespace app
} // namespace chip
