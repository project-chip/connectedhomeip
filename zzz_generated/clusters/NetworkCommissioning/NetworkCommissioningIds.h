// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster NetworkCommissioning (cluster code: 49/0x31)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace NetworkCommissioning {

inline constexpr ClusterId kClusterId = 0x00000031;

namespace Attributes {
namespace MaxNetworks {
inline constexpr AttributeId Id = 0x00000000;
} // namespace MaxNetworks
namespace Networks {
inline constexpr AttributeId Id = 0x00000001;
} // namespace Networks
namespace ScanMaxTimeSeconds {
inline constexpr AttributeId Id = 0x00000002;
} // namespace ScanMaxTimeSeconds
namespace ConnectMaxTimeSeconds {
inline constexpr AttributeId Id = 0x00000003;
} // namespace ConnectMaxTimeSeconds
namespace InterfaceEnabled {
inline constexpr AttributeId Id = 0x00000004;
} // namespace InterfaceEnabled
namespace LastNetworkingStatus {
inline constexpr AttributeId Id = 0x00000005;
} // namespace LastNetworkingStatus
namespace LastNetworkID {
inline constexpr AttributeId Id = 0x00000006;
} // namespace LastNetworkID
namespace LastConnectErrorValue {
inline constexpr AttributeId Id = 0x00000007;
} // namespace LastConnectErrorValue
namespace SupportedWiFiBands {
inline constexpr AttributeId Id = 0x00000008;
} // namespace SupportedWiFiBands
namespace SupportedThreadFeatures {
inline constexpr AttributeId Id = 0x00000009;
} // namespace SupportedThreadFeatures
namespace ThreadVersion {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace ThreadVersion

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
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
} // namespace Commands

namespace Events {} // namespace Events
} // namespace NetworkCommissioning
} // namespace clusters
} // namespace app
} // namespace chip
