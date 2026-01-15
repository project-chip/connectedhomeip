// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster CommissioningProxy (cluster code: 1109/0x455)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 6;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 3;

namespace ProxyConnectRequest {
inline constexpr CommandId Id = 0x00000000;
} // namespace ProxyConnectRequest

namespace ProxyDisconnectRequest {
inline constexpr CommandId Id = 0x00000002;
} // namespace ProxyDisconnectRequest

namespace ProxyScanRequest {
inline constexpr CommandId Id = 0x00000003;
} // namespace ProxyScanRequest

namespace ProxyBackGroundScanStartRequest {
inline constexpr CommandId Id = 0x00000005;
} // namespace ProxyBackGroundScanStartRequest

namespace ProxyBackGroundScanStopRequest {
inline constexpr CommandId Id = 0x00000006;
} // namespace ProxyBackGroundScanStopRequest

namespace ProxyMessageRequest {
inline constexpr CommandId Id = 0x00000007;
} // namespace ProxyMessageRequest

namespace ProxyConnectResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ProxyConnectResponse

namespace ProxyScanResponse {
inline constexpr CommandId Id = 0x00000004;
} // namespace ProxyScanResponse

namespace ProxyMessageResponse {
inline constexpr CommandId Id = 0x00000008;
} // namespace ProxyMessageResponse

} // namespace Commands
} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
