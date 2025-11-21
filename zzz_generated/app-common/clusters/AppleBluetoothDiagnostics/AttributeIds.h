// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AppleBluetoothDiagnostics (cluster code: 323615751/0x1349FC07)
// based on ../../../connectedhomeip/src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AppleBluetoothDiagnostics {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 20;

namespace AppleBluetoothVersion {
inline constexpr AttributeId Id = 0x00000000;
} // namespace AppleBluetoothVersion

namespace AppleBluetoothRSSI {
inline constexpr AttributeId Id = 0x00000001;
} // namespace AppleBluetoothRSSI

namespace AppleBluetoothTxPowerLevel {
inline constexpr AttributeId Id = 0x00000002;
} // namespace AppleBluetoothTxPowerLevel

namespace AppleBluetoothPacketRxCount {
inline constexpr AttributeId Id = 0x00000003;
} // namespace AppleBluetoothPacketRxCount

namespace AppleBluetoothPacketTxCount {
inline constexpr AttributeId Id = 0x00000004;
} // namespace AppleBluetoothPacketTxCount

namespace AppleBluetoothPacketRxErrorCount {
inline constexpr AttributeId Id = 0x00000005;
} // namespace AppleBluetoothPacketRxErrorCount

namespace AppleBluetoothPacketTxErrorCount {
inline constexpr AttributeId Id = 0x00000006;
} // namespace AppleBluetoothPacketTxErrorCount

namespace AppleBluetoothOverrunCount {
inline constexpr AttributeId Id = 0x00000007;
} // namespace AppleBluetoothOverrunCount

namespace AppleBluetoothConnectionStatus {
inline constexpr AttributeId Id = 0x00000008;
} // namespace AppleBluetoothConnectionStatus

namespace AppleBluetoothConnectionCount {
inline constexpr AttributeId Id = 0x00000009;
} // namespace AppleBluetoothConnectionCount

namespace AppleBluetoothConnectionErrorCount {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace AppleBluetoothConnectionErrorCount

namespace AppleBluetoothDisconnectionCount {
inline constexpr AttributeId Id = 0x0000000B;
} // namespace AppleBluetoothDisconnectionCount

namespace AppleBluetoothDisconnectionErrorCount {
inline constexpr AttributeId Id = 0x0000000C;
} // namespace AppleBluetoothDisconnectionErrorCount

namespace AppleBluetoothHardwareExceptionCount {
inline constexpr AttributeId Id = 0x0000000D;
} // namespace AppleBluetoothHardwareExceptionCount

namespace AppleBluetoothResetCountBootRelativeTime {
inline constexpr AttributeId Id = 0x0000000E;
} // namespace AppleBluetoothResetCountBootRelativeTime

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
} // namespace AppleBluetoothDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
