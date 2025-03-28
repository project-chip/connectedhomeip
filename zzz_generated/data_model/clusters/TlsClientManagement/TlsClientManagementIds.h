// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster TlsClientManagement (cluster code: 2050/0x802)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TlsClientManagement {

inline constexpr ClusterId Id = 0x00000802;

namespace Attributes {
namespace MaxProvisioned {
inline constexpr AttributeId Id = 0x00000000;
} // namespace MaxProvisioned
namespace ProvisionedEndpoints {
inline constexpr AttributeId Id = 0x00000001;
} // namespace ProvisionedEndpoints
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
namespace ProvisionEndpoint {
inline constexpr CommandId Id = 0x00000000;
} // namespace ProvisionEndpoint
namespace FindEndpoint {
inline constexpr CommandId Id = 0x00000002;
} // namespace FindEndpoint
namespace RemoveEndpoint {
inline constexpr CommandId Id = 0x00000004;
} // namespace RemoveEndpoint
namespace ProvisionEndpointResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ProvisionEndpointResponse
namespace FindEndpointResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace FindEndpointResponse
} // namespace Commands

namespace Events {} // namespace Events

} // namespace TlsClientManagement
} // namespace Clusters
} // namespace app
} // namespace chip
