// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ScenesManagement (cluster code: 98/0x62)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ScenesManagement {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 8;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 7;

namespace AddScene {
inline constexpr CommandId Id = 0x00000000;
} // namespace AddScene

namespace ViewScene {
inline constexpr CommandId Id = 0x00000001;
} // namespace ViewScene

namespace RemoveScene {
inline constexpr CommandId Id = 0x00000002;
} // namespace RemoveScene

namespace RemoveAllScenes {
inline constexpr CommandId Id = 0x00000003;
} // namespace RemoveAllScenes

namespace StoreScene {
inline constexpr CommandId Id = 0x00000004;
} // namespace StoreScene

namespace RecallScene {
inline constexpr CommandId Id = 0x00000005;
} // namespace RecallScene

namespace GetSceneMembership {
inline constexpr CommandId Id = 0x00000006;
} // namespace GetSceneMembership

namespace CopyScene {
inline constexpr CommandId Id = 0x00000040;
} // namespace CopyScene

namespace AddSceneResponse {
inline constexpr CommandId Id = 0x00000000;
} // namespace AddSceneResponse

namespace ViewSceneResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ViewSceneResponse

namespace RemoveSceneResponse {
inline constexpr CommandId Id = 0x00000002;
} // namespace RemoveSceneResponse

namespace RemoveAllScenesResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace RemoveAllScenesResponse

namespace StoreSceneResponse {
inline constexpr CommandId Id = 0x00000004;
} // namespace StoreSceneResponse

namespace GetSceneMembershipResponse {
inline constexpr CommandId Id = 0x00000006;
} // namespace GetSceneMembershipResponse

namespace CopySceneResponse {
inline constexpr CommandId Id = 0x00000040;
} // namespace CopySceneResponse

} // namespace Commands
} // namespace ScenesManagement
} // namespace Clusters
} // namespace app
} // namespace chip
