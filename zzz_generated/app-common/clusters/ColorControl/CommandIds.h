// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ColorControl (cluster code: 768/0x300)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ColorControl {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 19;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace MoveToHue {
inline constexpr CommandId Id = 0x00000000;
} // namespace MoveToHue

namespace MoveHue {
inline constexpr CommandId Id = 0x00000001;
} // namespace MoveHue

namespace StepHue {
inline constexpr CommandId Id = 0x00000002;
} // namespace StepHue

namespace MoveToSaturation {
inline constexpr CommandId Id = 0x00000003;
} // namespace MoveToSaturation

namespace MoveSaturation {
inline constexpr CommandId Id = 0x00000004;
} // namespace MoveSaturation

namespace StepSaturation {
inline constexpr CommandId Id = 0x00000005;
} // namespace StepSaturation

namespace MoveToHueAndSaturation {
inline constexpr CommandId Id = 0x00000006;
} // namespace MoveToHueAndSaturation

namespace MoveToColor {
inline constexpr CommandId Id = 0x00000007;
} // namespace MoveToColor

namespace MoveColor {
inline constexpr CommandId Id = 0x00000008;
} // namespace MoveColor

namespace StepColor {
inline constexpr CommandId Id = 0x00000009;
} // namespace StepColor

namespace MoveToColorTemperature {
inline constexpr CommandId Id = 0x0000000A;
} // namespace MoveToColorTemperature

namespace EnhancedMoveToHue {
inline constexpr CommandId Id = 0x00000040;
} // namespace EnhancedMoveToHue

namespace EnhancedMoveHue {
inline constexpr CommandId Id = 0x00000041;
} // namespace EnhancedMoveHue

namespace EnhancedStepHue {
inline constexpr CommandId Id = 0x00000042;
} // namespace EnhancedStepHue

namespace EnhancedMoveToHueAndSaturation {
inline constexpr CommandId Id = 0x00000043;
} // namespace EnhancedMoveToHueAndSaturation

namespace ColorLoopSet {
inline constexpr CommandId Id = 0x00000044;
} // namespace ColorLoopSet

namespace StopMoveStep {
inline constexpr CommandId Id = 0x00000047;
} // namespace StopMoveStep

namespace MoveColorTemperature {
inline constexpr CommandId Id = 0x0000004B;
} // namespace MoveColorTemperature

namespace StepColorTemperature {
inline constexpr CommandId Id = 0x0000004C;
} // namespace StepColorTemperature

} // namespace Commands
} // namespace ColorControl
} // namespace Clusters
} // namespace app
} // namespace chip
