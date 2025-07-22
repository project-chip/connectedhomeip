// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster NetworkCommissioning (cluster code: 49/0x31)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 7;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 4;

namespace ScanNetworks {
inline constexpr CommandId Id = 0x00000000;
} // namespace ScanNetworks

namespace AddOrUpdateWiFiNetwork {
inline constexpr CommandId Id = 0x00000002;
} // namespace AddOrUpdateWiFiNetwork

namespace AddOrUpdateThreadNetwork {
inline constexpr CommandId Id = 0x00000003;
} // namespace AddOrUpdateThreadNetwork

namespace RemoveNetwork {
inline constexpr CommandId Id = 0x00000004;
} // namespace RemoveNetwork

namespace ConnectNetwork {
inline constexpr CommandId Id = 0x00000006;
} // namespace ConnectNetwork

namespace ReorderNetwork {
inline constexpr CommandId Id = 0x00000008;
} // namespace ReorderNetwork

namespace QueryIdentity {
inline constexpr CommandId Id = 0x00000009;
} // namespace QueryIdentity

namespace ScanNetworksResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ScanNetworksResponse

namespace NetworkConfigResponse {
inline constexpr CommandId Id = 0x00000005;
} // namespace NetworkConfigResponse

namespace ConnectNetworkResponse {
inline constexpr CommandId Id = 0x00000007;
} // namespace ConnectNetworkResponse

namespace QueryIdentityResponse {
inline constexpr CommandId Id = 0x0000000A;
} // namespace QueryIdentityResponse

} // namespace Commands
} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
