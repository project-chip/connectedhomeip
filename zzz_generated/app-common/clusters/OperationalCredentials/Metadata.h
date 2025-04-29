// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OperationalCredentials (cluster code: 62/0x3E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/OperationalCredentials/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalCredentials {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace NOCs {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NOCs::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kAdminister,
    .writePrivilege = std::nullopt,
};
} // namespace NOCs
namespace Fabrics {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Fabrics::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Fabrics
namespace SupportedFabrics {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SupportedFabrics::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedFabrics
namespace CommissionedFabrics {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CommissionedFabrics::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CommissionedFabrics
namespace TrustedRootCertificates {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TrustedRootCertificates::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TrustedRootCertificates
namespace CurrentFabricIndex {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentFabricIndex::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentFabricIndex

} // namespace Attributes

namespace Commands {
namespace AttestationRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AttestationRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AttestationRequest
namespace CertificateChainRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CertificateChainRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace CertificateChainRequest
namespace CSRRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = CSRRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace CSRRequest
namespace AddNOC {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AddNOC::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AddNOC
namespace UpdateNOC {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = UpdateNOC::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace UpdateNOC
namespace UpdateFabricLabel {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = UpdateFabricLabel::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace UpdateFabricLabel
namespace RemoveFabric {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = RemoveFabric::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace RemoveFabric
namespace AddTrustedRootCertificate {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AddTrustedRootCertificate::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace AddTrustedRootCertificate
namespace SetVIDVerificationStatement {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetVIDVerificationStatement::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SetVIDVerificationStatement
namespace SignVIDVerificationRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SignVIDVerificationRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SignVIDVerificationRequest

} // namespace Commands
} // namespace OperationalCredentials
} // namespace Clusters
} // namespace app
} // namespace chip
