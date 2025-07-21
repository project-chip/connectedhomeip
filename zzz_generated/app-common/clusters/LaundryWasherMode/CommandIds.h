// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster LaundryWasherMode (cluster code: 81/0x51)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LaundryWasherMode {
namespace Commands {

inline constexpr uint32_t kAcceptedCommandsCount  = 1; // Total number of commands without a response supported by the cluster
inline constexpr uint32_t kGeneratedCommandsCount = 1; // Total number of commands with a response supported by the cluster

namespace ChangeToMode {
inline constexpr CommandId Id = 0x00000000;
} // namespace ChangeToMode

namespace ChangeToModeResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ChangeToModeResponse

} // namespace Commands
} // namespace LaundryWasherMode
} // namespace Clusters
} // namespace app
} // namespace chip
