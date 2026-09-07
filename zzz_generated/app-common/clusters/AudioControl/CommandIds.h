// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AudioControl (cluster code: 1298/0x512)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AudioControl {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 6;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace Mute {
inline constexpr CommandId Id = 0x00000000;
} // namespace Mute

namespace Unmute {
inline constexpr CommandId Id = 0x00000001;
} // namespace Unmute

namespace ToggleMuted {
inline constexpr CommandId Id = 0x00000002;
} // namespace ToggleMuted

namespace SetVolume {
inline constexpr CommandId Id = 0x00000003;
} // namespace SetVolume

namespace IncreaseVolume {
inline constexpr CommandId Id = 0x00000004;
} // namespace IncreaseVolume

namespace DecreaseVolume {
inline constexpr CommandId Id = 0x00000005;
} // namespace DecreaseVolume

} // namespace Commands
} // namespace AudioControl
} // namespace Clusters
} // namespace app
} // namespace chip
