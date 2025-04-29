// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TlsCertificateManagement (cluster code: 2049/0x801)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TlsCertificateManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TlsCertificateManagement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace MaxRootCertificates {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxRootCertificates::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxRootCertificates
namespace ProvisionedRootCertificates {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ProvisionedRootCertificates::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ProvisionedRootCertificates
namespace MaxClientCertificates {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxClientCertificates::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxClientCertificates
namespace ProvisionedClientCertificates {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ProvisionedClientCertificates::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ProvisionedClientCertificates

} // namespace Attributes

namespace Commands {
namespace ProvisionRootCertificate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ProvisionRootCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ProvisionRootCertificate
namespace FindRootCertificate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = FindRootCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace FindRootCertificate
namespace LookupRootCertificate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = LookupRootCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace LookupRootCertificate
namespace RemoveRootCertificate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RemoveRootCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace RemoveRootCertificate
namespace TLSClientCSR {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = TLSClientCSR::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace TLSClientCSR
namespace ProvisionClientCertificate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ProvisionClientCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace ProvisionClientCertificate
namespace FindClientCertificate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = FindClientCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace FindClientCertificate
namespace LookupClientCertificate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = LookupClientCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace LookupClientCertificate
namespace RemoveClientCertificate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RemoveClientCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace RemoveClientCertificate

} // namespace Commands
} // namespace TlsCertificateManagement
} // namespace Clusters
} // namespace app
} // namespace chip
