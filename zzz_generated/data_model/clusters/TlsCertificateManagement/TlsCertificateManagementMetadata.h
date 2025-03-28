// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TlsCertificateManagement (cluster code: 2049/0x801)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TlsCertificateManagement/TlsCertificateManagementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace TlsCertificateManagement {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kMaxRootCertificatesEntry = {
    .attributeId    = TlsCertificateManagement::Attributes::MaxRootCertificates::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kProvisionedRootCertificatesEntry = {
    .attributeId    = TlsCertificateManagement::Attributes::ProvisionedRootCertificates::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxClientCertificatesEntry = {
    .attributeId    = TlsCertificateManagement::Attributes::MaxClientCertificates::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kProvisionedClientCertificatesEntry = {
    .attributeId    = TlsCertificateManagement::Attributes::ProvisionedClientCertificates::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kProvisionRootCertificateEntry = {
    .commandId       = TlsCertificateManagement::Commands::ProvisionRootCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kFindRootCertificateEntry = {
    .commandId       = TlsCertificateManagement::Commands::FindRootCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kLookupRootCertificateEntry = {
    .commandId       = TlsCertificateManagement::Commands::LookupRootCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kRemoveRootCertificateEntry = {
    .commandId       = TlsCertificateManagement::Commands::RemoveRootCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kTLSClientCSREntry = {
    .commandId       = TlsCertificateManagement::Commands::TLSClientCSR::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kProvisionClientCertificateEntry = {
    .commandId       = TlsCertificateManagement::Commands::ProvisionClientCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kFindClientCertificateEntry = {
    .commandId       = TlsCertificateManagement::Commands::FindClientCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kLookupClientCertificateEntry = {
    .commandId       = TlsCertificateManagement::Commands::LookupClientCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kRemoveClientCertificateEntry = {
    .commandId       = TlsCertificateManagement::Commands::RemoveClientCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};

} // namespace Commands
} // namespace Metadata
} // namespace TlsCertificateManagement
} // namespace clusters
} // namespace app
} // namespace chip
