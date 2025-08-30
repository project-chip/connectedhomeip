// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster CameraAvStreamManagement (cluster code: 1361/0x551)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvStreamManagement {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 10;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 4;

namespace AudioStreamAllocate {
inline constexpr CommandId Id = 0x00000000;
} // namespace AudioStreamAllocate

namespace AudioStreamDeallocate {
inline constexpr CommandId Id = 0x00000002;
} // namespace AudioStreamDeallocate

namespace VideoStreamAllocate {
inline constexpr CommandId Id = 0x00000003;
} // namespace VideoStreamAllocate

namespace VideoStreamModify {
inline constexpr CommandId Id = 0x00000005;
} // namespace VideoStreamModify

namespace VideoStreamDeallocate {
inline constexpr CommandId Id = 0x00000006;
} // namespace VideoStreamDeallocate

namespace SnapshotStreamAllocate {
inline constexpr CommandId Id = 0x00000007;
} // namespace SnapshotStreamAllocate

namespace SnapshotStreamModify {
inline constexpr CommandId Id = 0x00000009;
} // namespace SnapshotStreamModify

namespace SnapshotStreamDeallocate {
inline constexpr CommandId Id = 0x0000000A;
} // namespace SnapshotStreamDeallocate

namespace SetStreamPriorities {
inline constexpr CommandId Id = 0x0000000B;
} // namespace SetStreamPriorities

namespace CaptureSnapshot {
inline constexpr CommandId Id = 0x0000000C;
} // namespace CaptureSnapshot

namespace AudioStreamAllocateResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace AudioStreamAllocateResponse

namespace VideoStreamAllocateResponse {
inline constexpr CommandId Id = 0x00000004;
} // namespace VideoStreamAllocateResponse

namespace SnapshotStreamAllocateResponse {
inline constexpr CommandId Id = 0x00000008;
} // namespace SnapshotStreamAllocateResponse

namespace CaptureSnapshotResponse {
inline constexpr CommandId Id = 0x0000000D;
} // namespace CaptureSnapshotResponse

} // namespace Commands
} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
