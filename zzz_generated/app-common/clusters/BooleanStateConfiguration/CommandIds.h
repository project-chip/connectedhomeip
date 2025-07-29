// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster BooleanStateConfiguration (cluster code: 128/0x80)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace BooleanStateConfiguration {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 2;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace SuppressAlarm {
inline constexpr CommandId Id = 0x00000000;
} // namespace SuppressAlarm

namespace EnableDisableAlarm {
inline constexpr CommandId Id = 0x00000001;
} // namespace EnableDisableAlarm

} // namespace Commands
} // namespace BooleanStateConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip
