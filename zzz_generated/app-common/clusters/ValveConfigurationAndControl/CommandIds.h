// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ValveConfigurationAndControl (cluster code: 129/0x81)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ValveConfigurationAndControl {
namespace Commands {

inline constexpr uint32_t kAcceptedCommandsCount  = 2; // Total number of commands without a response supported by the cluster
inline constexpr uint32_t kGeneratedCommandsCount = 0; // Total number of commands with a response supported by the cluster

namespace Open {
inline constexpr CommandId Id = 0x00000000;
} // namespace Open

namespace Close {
inline constexpr CommandId Id = 0x00000001;
} // namespace Close

} // namespace Commands
} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
