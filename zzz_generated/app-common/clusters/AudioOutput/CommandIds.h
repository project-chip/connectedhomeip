// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AudioOutput (cluster code: 1291/0x50B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AudioOutput {
namespace Commands {

inline constexpr uint32_t kAcceptedCommandsCount  = 2; // Total number of commands without a response supported by the cluster
inline constexpr uint32_t kGeneratedCommandsCount = 0; // Total number of commands with a response supported by the cluster

namespace SelectOutput {
inline constexpr CommandId Id = 0x00000000;
} // namespace SelectOutput

namespace RenameOutput {
inline constexpr CommandId Id = 0x00000001;
} // namespace RenameOutput

} // namespace Commands
} // namespace AudioOutput
} // namespace Clusters
} // namespace app
} // namespace chip
