// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster PushAvStreamTransport (cluster code: 1365/0x555)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 6;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 2;

namespace AllocatePushTransport {
inline constexpr CommandId Id = 0x00000000;
} // namespace AllocatePushTransport

namespace DeallocatePushTransport {
inline constexpr CommandId Id = 0x00000002;
} // namespace DeallocatePushTransport

namespace ModifyPushTransport {
inline constexpr CommandId Id = 0x00000003;
} // namespace ModifyPushTransport

namespace SetTransportStatus {
inline constexpr CommandId Id = 0x00000004;
} // namespace SetTransportStatus

namespace ManuallyTriggerTransport {
inline constexpr CommandId Id = 0x00000005;
} // namespace ManuallyTriggerTransport

namespace FindTransport {
inline constexpr CommandId Id = 0x00000006;
} // namespace FindTransport

namespace AllocatePushTransportResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace AllocatePushTransportResponse

namespace FindTransportResponse {
inline constexpr CommandId Id = 0x00000007;
} // namespace FindTransportResponse

} // namespace Commands
} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
