// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster TlsCertificateManagement (cluster code: 2049/0x801)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TlsCertificateManagement {
namespace Attributes {
namespace MaxRootCertificates {
inline constexpr AttributeId Id = 0x00000000;
} // namespace MaxRootCertificates

namespace ProvisionedRootCertificates {
inline constexpr AttributeId Id = 0x00000001;
} // namespace ProvisionedRootCertificates

namespace MaxClientCertificates {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MaxClientCertificates

namespace ProvisionedClientCertificates {
inline constexpr AttributeId Id = 0x00000003;
} // namespace ProvisionedClientCertificates

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
} // namespace TlsCertificateManagement
} // namespace Clusters
} // namespace app
} // namespace chip
