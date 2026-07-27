// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster MediaInput (cluster code: 1287/0x507)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaInput {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 4;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace SelectInput {
inline constexpr CommandId Id = 0x00000000;
} // namespace SelectInput

namespace ShowInputStatus {
inline constexpr CommandId Id = 0x00000001;
} // namespace ShowInputStatus

namespace HideInputStatus {
inline constexpr CommandId Id = 0x00000002;
} // namespace HideInputStatus

namespace RenameInput {
inline constexpr CommandId Id = 0x00000003;
} // namespace RenameInput

} // namespace Commands
} // namespace MediaInput
} // namespace Clusters
} // namespace app
} // namespace chip
