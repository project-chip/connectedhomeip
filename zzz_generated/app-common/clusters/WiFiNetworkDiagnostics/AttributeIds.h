// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster WiFiNetworkDiagnostics (cluster code: 54/0x36)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WiFiNetworkDiagnostics {
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

namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList

namespace AttributeList {
inline constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList

namespace FeatureMap {
inline constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap

namespace ClusterRevision {
inline constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

} // namespace Attributes
} // namespace WiFiNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
