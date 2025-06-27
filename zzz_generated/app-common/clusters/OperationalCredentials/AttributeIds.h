// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OperationalCredentials (cluster code: 62/0x3E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalCredentials {
namespace Attributes {
namespace NOCs {
inline constexpr AttributeId Id = 0x00000000;
} // namespace NOCs

namespace Fabrics {
inline constexpr AttributeId Id = 0x00000001;
} // namespace Fabrics

namespace SupportedFabrics {
inline constexpr AttributeId Id = 0x00000002;
} // namespace SupportedFabrics

namespace CommissionedFabrics {
inline constexpr AttributeId Id = 0x00000003;
} // namespace CommissionedFabrics

namespace TrustedRootCertificates {
inline constexpr AttributeId Id = 0x00000004;
} // namespace TrustedRootCertificates

namespace CurrentFabricIndex {
inline constexpr AttributeId Id = 0x00000005;
} // namespace CurrentFabricIndex

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
} // namespace OperationalCredentials
} // namespace Clusters
} // namespace app
} // namespace chip
