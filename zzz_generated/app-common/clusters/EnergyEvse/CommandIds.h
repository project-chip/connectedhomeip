// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster EnergyEvse (cluster code: 153/0x99)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 7;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 1;

namespace Disable {
inline constexpr CommandId Id = 0x00000001;
} // namespace Disable

namespace EnableCharging {
inline constexpr CommandId Id = 0x00000002;
} // namespace EnableCharging

namespace EnableDischarging {
inline constexpr CommandId Id = 0x00000003;
} // namespace EnableDischarging

namespace StartDiagnostics {
inline constexpr CommandId Id = 0x00000004;
} // namespace StartDiagnostics

namespace SetTargets {
inline constexpr CommandId Id = 0x00000005;
} // namespace SetTargets

namespace GetTargets {
inline constexpr CommandId Id = 0x00000006;
} // namespace GetTargets

namespace ClearTargets {
inline constexpr CommandId Id = 0x00000007;
} // namespace ClearTargets

namespace GetTargetsResponse {
inline constexpr CommandId Id = 0x00000000;
} // namespace GetTargetsResponse

} // namespace Commands
} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
