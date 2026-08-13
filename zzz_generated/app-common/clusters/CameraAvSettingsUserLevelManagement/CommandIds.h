// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster CameraAvSettingsUserLevelManagement (cluster code: 1362/0x552)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvSettingsUserLevelManagement {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 7;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace MPTZSetPosition {
inline constexpr CommandId Id = 0x00000000;
} // namespace MPTZSetPosition

namespace MPTZRelativeMove {
inline constexpr CommandId Id = 0x00000001;
} // namespace MPTZRelativeMove

namespace MPTZMoveToPreset {
inline constexpr CommandId Id = 0x00000002;
} // namespace MPTZMoveToPreset

namespace MPTZSavePreset {
inline constexpr CommandId Id = 0x00000003;
} // namespace MPTZSavePreset

namespace MPTZRemovePreset {
inline constexpr CommandId Id = 0x00000004;
} // namespace MPTZRemovePreset

namespace DPTZSetViewport {
inline constexpr CommandId Id = 0x00000005;
} // namespace DPTZSetViewport

namespace DPTZRelativeMove {
inline constexpr CommandId Id = 0x00000006;
} // namespace DPTZRelativeMove

} // namespace Commands
} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip
