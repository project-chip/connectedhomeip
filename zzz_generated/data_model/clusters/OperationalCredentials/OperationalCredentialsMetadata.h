// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OperationalCredentials (cluster code: 62/0x3E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/OperationalCredentials/OperationalCredentialsIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace OperationalCredentials {
namespace Metadata {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kNOCsEntry = {
    .attributeId    = OperationalCredentials::Attributes::NOCs::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kFabricsEntry = {
    .attributeId    = OperationalCredentials::Attributes::Fabrics::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSupportedFabricsEntry = {
    .attributeId    = OperationalCredentials::Attributes::SupportedFabrics::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCommissionedFabricsEntry = {
    .attributeId    = OperationalCredentials::Attributes::CommissionedFabrics::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTrustedRootCertificatesEntry = {
    .attributeId    = OperationalCredentials::Attributes::TrustedRootCertificates::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentFabricIndexEntry = {
    .attributeId    = OperationalCredentials::Attributes::CurrentFabricIndex::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kAttestationRequestEntry = {
    .commandId       = OperationalCredentials::Commands::AttestationRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kCertificateChainRequestEntry = {
    .commandId       = OperationalCredentials::Commands::CertificateChainRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kCSRRequestEntry = {
    .commandId       = OperationalCredentials::Commands::CSRRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kAddNOCEntry = {
    .commandId       = OperationalCredentials::Commands::AddNOC::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kUpdateNOCEntry = {
    .commandId       = OperationalCredentials::Commands::UpdateNOC::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kUpdateFabricLabelEntry = {
    .commandId       = OperationalCredentials::Commands::UpdateFabricLabel::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kRemoveFabricEntry = {
    .commandId       = OperationalCredentials::Commands::RemoveFabric::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kAddTrustedRootCertificateEntry = {
    .commandId       = OperationalCredentials::Commands::AddTrustedRootCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kSetVIDVerificationStatementEntry = {
    .commandId       = OperationalCredentials::Commands::SetVIDVerificationStatement::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kSignVIDVerificationRequestEntry = {
    .commandId       = OperationalCredentials::Commands::SignVIDVerificationRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};

} // namespace Commands
} // namespace Metadata
} // namespace OperationalCredentials
} // namespace clusters
} // namespace app
} // namespace chip
