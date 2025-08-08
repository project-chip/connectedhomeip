// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OnOff (cluster code: 6/0x6)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OnOff {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 6;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace Off {
inline constexpr CommandId Id = 0x00000000;
} // namespace Off

namespace On {
inline constexpr CommandId Id = 0x00000001;
} // namespace On

namespace Toggle {
inline constexpr CommandId Id = 0x00000002;
} // namespace Toggle

namespace OffWithEffect {
inline constexpr CommandId Id = 0x00000040;
} // namespace OffWithEffect

namespace OnWithRecallGlobalScene {
inline constexpr CommandId Id = 0x00000041;
} // namespace OnWithRecallGlobalScene

namespace OnWithTimedOff {
inline constexpr CommandId Id = 0x00000042;
} // namespace OnWithTimedOff

} // namespace Commands
} // namespace OnOff
} // namespace Clusters
} // namespace app
} // namespace chip
