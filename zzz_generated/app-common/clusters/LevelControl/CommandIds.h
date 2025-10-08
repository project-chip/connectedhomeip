// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster LevelControl (cluster code: 8/0x8)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LevelControl {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 9;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace MoveToLevel {
inline constexpr CommandId Id = 0x00000000;
} // namespace MoveToLevel

namespace Move {
inline constexpr CommandId Id = 0x00000001;
} // namespace Move

namespace Step {
inline constexpr CommandId Id = 0x00000002;
} // namespace Step

namespace Stop {
inline constexpr CommandId Id = 0x00000003;
} // namespace Stop

namespace MoveToLevelWithOnOff {
inline constexpr CommandId Id = 0x00000004;
} // namespace MoveToLevelWithOnOff

namespace MoveWithOnOff {
inline constexpr CommandId Id = 0x00000005;
} // namespace MoveWithOnOff

namespace StepWithOnOff {
inline constexpr CommandId Id = 0x00000006;
} // namespace StepWithOnOff

namespace StopWithOnOff {
inline constexpr CommandId Id = 0x00000007;
} // namespace StopWithOnOff

namespace MoveToClosestFrequency {
inline constexpr CommandId Id = 0x00000008;
} // namespace MoveToClosestFrequency

} // namespace Commands
} // namespace LevelControl
} // namespace Clusters
} // namespace app
} // namespace chip
