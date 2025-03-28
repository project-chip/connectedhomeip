// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster WiFiNetworkDiagnostics (cluster code: 54/0x36)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace WiFiNetworkDiagnostics {

inline constexpr ClusterId kClusterId = 0x00000036;

namespace Attributes {
namespace Bssid {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Bssid
namespace SecurityType {
inline constexpr AttributeId Id = 0x00000001;
} // namespace SecurityType
namespace WiFiVersion {
inline constexpr AttributeId Id = 0x00000002;
} // namespace WiFiVersion
namespace ChannelNumber {
inline constexpr AttributeId Id = 0x00000003;
} // namespace ChannelNumber
namespace Rssi {
inline constexpr AttributeId Id = 0x00000004;
} // namespace Rssi
namespace BeaconLostCount {
inline constexpr AttributeId Id = 0x00000005;
} // namespace BeaconLostCount
namespace BeaconRxCount {
inline constexpr AttributeId Id = 0x00000006;
} // namespace BeaconRxCount
namespace PacketMulticastRxCount {
inline constexpr AttributeId Id = 0x00000007;
} // namespace PacketMulticastRxCount
namespace PacketMulticastTxCount {
inline constexpr AttributeId Id = 0x00000008;
} // namespace PacketMulticastTxCount
namespace PacketUnicastRxCount {
inline constexpr AttributeId Id = 0x00000009;
} // namespace PacketUnicastRxCount
namespace PacketUnicastTxCount {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace PacketUnicastTxCount
namespace CurrentMaxRate {
inline constexpr AttributeId Id = 0x0000000B;
} // namespace CurrentMaxRate
namespace OverrunCount {
inline constexpr AttributeId Id = 0x0000000C;
} // namespace OverrunCount

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace ResetCounts {
inline constexpr CommandId Id = 0x00000000;
} // namespace ResetCounts
} // namespace Commands

namespace Events {
namespace Disconnection {
inline constexpr EventId Id = 0x00000000;
} // namespace Disconnection
namespace AssociationFailure {
inline constexpr EventId Id = 0x00000001;
} // namespace AssociationFailure
namespace ConnectionStatus {
inline constexpr EventId Id = 0x00000002;
} // namespace ConnectionStatus
} // namespace Events
} // namespace WiFiNetworkDiagnostics
} // namespace clusters
} // namespace app
} // namespace chip
