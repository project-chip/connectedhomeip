// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster NetworkCommissioning (cluster code: 49/0x31)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

inline constexpr ClusterId Id = 0x00000031;

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

namespace Events {} // namespace Events

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
