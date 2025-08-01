// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster DeviceEnergyManagement (cluster code: 152/0x98)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 8;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace PowerAdjustRequest {
inline constexpr CommandId Id = 0x00000000;
} // namespace PowerAdjustRequest

namespace CancelPowerAdjustRequest {
inline constexpr CommandId Id = 0x00000001;
} // namespace CancelPowerAdjustRequest

namespace StartTimeAdjustRequest {
inline constexpr CommandId Id = 0x00000002;
} // namespace StartTimeAdjustRequest

namespace PauseRequest {
inline constexpr CommandId Id = 0x00000003;
} // namespace PauseRequest

namespace ResumeRequest {
inline constexpr CommandId Id = 0x00000004;
} // namespace ResumeRequest

namespace ModifyForecastRequest {
inline constexpr CommandId Id = 0x00000005;
} // namespace ModifyForecastRequest

namespace RequestConstraintBasedForecast {
inline constexpr CommandId Id = 0x00000006;
} // namespace RequestConstraintBasedForecast

namespace CancelRequest {
inline constexpr CommandId Id = 0x00000007;
} // namespace CancelRequest

} // namespace Commands
} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
