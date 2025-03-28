// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster EthernetNetworkDiagnostics (cluster code: 55/0x37)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EthernetNetworkDiagnostics {

inline constexpr ClusterId Id = 0x00000037;

namespace Attributes {
namespace PHYRate {
inline constexpr AttributeId Id = 0x00000000;
} // namespace PHYRate
namespace FullDuplex {
inline constexpr AttributeId Id = 0x00000001;
} // namespace FullDuplex
namespace PacketRxCount {
inline constexpr AttributeId Id = 0x00000002;
} // namespace PacketRxCount
namespace PacketTxCount {
inline constexpr AttributeId Id = 0x00000003;
} // namespace PacketTxCount
namespace TxErrCount {
inline constexpr AttributeId Id = 0x00000004;
} // namespace TxErrCount
namespace CollisionCount {
inline constexpr AttributeId Id = 0x00000005;
} // namespace CollisionCount
namespace OverrunCount {
inline constexpr AttributeId Id = 0x00000006;
} // namespace OverrunCount
namespace CarrierDetect {
inline constexpr AttributeId Id = 0x00000007;
} // namespace CarrierDetect
namespace TimeSinceReset {
inline constexpr AttributeId Id = 0x00000008;
} // namespace TimeSinceReset
namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList
namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList
namespace EventList {
inline constexpr AttributeId Id = Globals::Attributes::EventList::Id;
} // namespace EventList
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

namespace Commands {
namespace ResetCounts {
inline constexpr CommandId Id = 0x00000000;
} // namespace ResetCounts
} // namespace Commands

namespace Events {} // namespace Events

} // namespace EthernetNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
