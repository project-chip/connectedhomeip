// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster MicrowaveOvenControl (cluster code: 95/0x5F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {
namespace Commands {

inline constexpr uint32_t kAcceptedCommandsCount  = 2; // Total number of commands without a response supported by the cluster
inline constexpr uint32_t kGeneratedCommandsCount = 0; // Total number of commands with a response supported by the cluster

namespace SetCookingParameters {
inline constexpr CommandId Id = 0x00000000;
} // namespace SetCookingParameters

namespace AddMoreTime {
inline constexpr CommandId Id = 0x00000001;
} // namespace AddMoreTime

} // namespace Commands
} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip
