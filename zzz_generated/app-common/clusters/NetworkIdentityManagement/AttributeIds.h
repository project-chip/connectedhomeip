// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster NetworkIdentityManagement (cluster code: 1104/0x450)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkIdentityManagement {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 8;

namespace ActiveNetworkIdentities {
inline constexpr AttributeId Id = 0x00000000;
} // namespace ActiveNetworkIdentities

namespace Clients {
inline constexpr AttributeId Id = 0x00000001;
} // namespace Clients

namespace ClientTableSize {
inline constexpr AttributeId Id = 0x00000002;
} // namespace ClientTableSize

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
} // namespace NetworkIdentityManagement
} // namespace Clusters
} // namespace app
} // namespace chip
