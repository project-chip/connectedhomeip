// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster GroupKeyManagement (cluster code: 63/0x3F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GroupKeyManagement {

inline constexpr ClusterId Id = 0x0000003F;

namespace Attributes {
namespace GroupKeyMap {
inline constexpr AttributeId Id = 0x00000000;
} // namespace GroupKeyMap
namespace GroupTable {
inline constexpr AttributeId Id = 0x00000001;
} // namespace GroupTable
namespace MaxGroupsPerFabric {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MaxGroupsPerFabric
namespace MaxGroupKeysPerFabric {
inline constexpr AttributeId Id = 0x00000003;
} // namespace MaxGroupKeysPerFabric
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
namespace KeySetWrite {
inline constexpr CommandId Id = 0x00000000;
} // namespace KeySetWrite
namespace KeySetRead {
inline constexpr CommandId Id = 0x00000001;
} // namespace KeySetRead
namespace KeySetRemove {
inline constexpr CommandId Id = 0x00000003;
} // namespace KeySetRemove
namespace KeySetReadAllIndices {
inline constexpr CommandId Id = 0x00000004;
} // namespace KeySetReadAllIndices
namespace KeySetReadResponse {
inline constexpr CommandId Id = 0x00000002;
} // namespace KeySetReadResponse
namespace KeySetReadAllIndicesResponse {
inline constexpr CommandId Id = 0x00000005;
} // namespace KeySetReadAllIndicesResponse
} // namespace Commands

namespace Events {} // namespace Events

} // namespace GroupKeyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
