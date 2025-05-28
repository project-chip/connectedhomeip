// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AccessControl (cluster code: 31/0x1F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AccessControl {
namespace Attributes {
namespace Acl {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Acl

namespace Extension {
inline constexpr AttributeId Id = 0x00000001;
} // namespace Extension

namespace SubjectsPerAccessControlEntry {
inline constexpr AttributeId Id = 0x00000002;
} // namespace SubjectsPerAccessControlEntry

namespace TargetsPerAccessControlEntry {
inline constexpr AttributeId Id = 0x00000003;
} // namespace TargetsPerAccessControlEntry

namespace AccessControlEntriesPerFabric {
inline constexpr AttributeId Id = 0x00000004;
} // namespace AccessControlEntriesPerFabric

namespace CommissioningARL {
inline constexpr AttributeId Id = 0x00000005;
} // namespace CommissioningARL

namespace Arl {
inline constexpr AttributeId Id = 0x00000006;
} // namespace Arl

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
} // namespace AccessControl
} // namespace Clusters
} // namespace app
} // namespace chip
