// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ThreadNetworkDirectory (cluster code: 1107/0x453)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadNetworkDirectory {

inline constexpr ClusterId Id = 0x00000453;

namespace Attributes {
namespace PreferredExtendedPanID {
inline constexpr AttributeId Id = 0x00000000;
} // namespace PreferredExtendedPanID
namespace ThreadNetworks {
inline constexpr AttributeId Id = 0x00000001;
} // namespace ThreadNetworks
namespace ThreadNetworkTableSize {
inline constexpr AttributeId Id = 0x00000002;
} // namespace ThreadNetworkTableSize
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
namespace AddNetwork {
inline constexpr CommandId Id = 0x00000000;
} // namespace AddNetwork
namespace RemoveNetwork {
inline constexpr CommandId Id = 0x00000001;
} // namespace RemoveNetwork
namespace GetOperationalDataset {
inline constexpr CommandId Id = 0x00000002;
} // namespace GetOperationalDataset
namespace OperationalDatasetResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace OperationalDatasetResponse
} // namespace Commands

namespace Events {} // namespace Events

} // namespace ThreadNetworkDirectory
} // namespace Clusters
} // namespace app
} // namespace chip
