// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster GroupKeyManagement (cluster code: 63/0x3F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GroupKeyManagement {
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
} // namespace GroupKeyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
