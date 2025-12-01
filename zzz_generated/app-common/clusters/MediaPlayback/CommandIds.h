// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster MediaPlayback (cluster code: 1286/0x506)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaPlayback {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 14;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace Play {
inline constexpr CommandId Id = 0x00000000;
} // namespace Play

namespace Pause {
inline constexpr CommandId Id = 0x00000001;
} // namespace Pause

namespace Stop {
inline constexpr CommandId Id = 0x00000002;
} // namespace Stop

namespace StartOver {
inline constexpr CommandId Id = 0x00000003;
} // namespace StartOver

namespace Previous {
inline constexpr CommandId Id = 0x00000004;
} // namespace Previous

namespace Next {
inline constexpr CommandId Id = 0x00000005;
} // namespace Next

namespace Rewind {
inline constexpr CommandId Id = 0x00000006;
} // namespace Rewind

namespace FastForward {
inline constexpr CommandId Id = 0x00000007;
} // namespace FastForward

namespace SkipForward {
inline constexpr CommandId Id = 0x00000008;
} // namespace SkipForward

namespace SkipBackward {
inline constexpr CommandId Id = 0x00000009;
} // namespace SkipBackward

namespace Seek {
inline constexpr CommandId Id = 0x0000000B;
} // namespace Seek

namespace ActivateAudioTrack {
inline constexpr CommandId Id = 0x0000000C;
} // namespace ActivateAudioTrack

namespace ActivateTextTrack {
inline constexpr CommandId Id = 0x0000000D;
} // namespace ActivateTextTrack

namespace DeactivateTextTrack {
inline constexpr CommandId Id = 0x0000000E;
} // namespace DeactivateTextTrack

namespace PlaybackResponse {
inline constexpr CommandId Id = 0x0000000A;
} // namespace PlaybackResponse

} // namespace Commands
} // namespace MediaPlayback
} // namespace Clusters
} // namespace app
} // namespace chip
