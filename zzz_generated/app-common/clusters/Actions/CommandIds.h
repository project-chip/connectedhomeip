// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Actions (cluster code: 37/0x25)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Actions {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 12;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace InstantAction {
inline constexpr CommandId Id = 0x00000000;
} // namespace InstantAction

namespace InstantActionWithTransition {
inline constexpr CommandId Id = 0x00000001;
} // namespace InstantActionWithTransition

namespace StartAction {
inline constexpr CommandId Id = 0x00000002;
} // namespace StartAction

namespace StartActionWithDuration {
inline constexpr CommandId Id = 0x00000003;
} // namespace StartActionWithDuration

namespace StopAction {
inline constexpr CommandId Id = 0x00000004;
} // namespace StopAction

namespace PauseAction {
inline constexpr CommandId Id = 0x00000005;
} // namespace PauseAction

namespace PauseActionWithDuration {
inline constexpr CommandId Id = 0x00000006;
} // namespace PauseActionWithDuration

namespace ResumeAction {
inline constexpr CommandId Id = 0x00000007;
} // namespace ResumeAction

namespace EnableAction {
inline constexpr CommandId Id = 0x00000008;
} // namespace EnableAction

namespace EnableActionWithDuration {
inline constexpr CommandId Id = 0x00000009;
} // namespace EnableActionWithDuration

namespace DisableAction {
inline constexpr CommandId Id = 0x0000000A;
} // namespace DisableAction

namespace DisableActionWithDuration {
inline constexpr CommandId Id = 0x0000000B;
} // namespace DisableActionWithDuration

} // namespace Commands
} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip
