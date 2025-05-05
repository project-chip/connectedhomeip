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
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(NOCs::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kAdminister, std::nullopt);
} // namespace NOCs
namespace Fabrics {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Fabrics::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace Fabrics
namespace SupportedFabrics {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SupportedFabrics::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SupportedFabrics
namespace CommissionedFabrics {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CommissionedFabrics::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CommissionedFabrics
namespace TrustedRootCertificates {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(TrustedRootCertificates::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace TrustedRootCertificates
namespace CurrentFabricIndex {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentFabricIndex::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentFabricIndex

} // namespace Attributes

namespace Commands {
namespace AttestationRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AttestationRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace AttestationRequest
namespace CertificateChainRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(CertificateChainRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace CertificateChainRequest
namespace CSRRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(CSRRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace CSRRequest
namespace AddNOC {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AddNOC::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace AddNOC
namespace UpdateNOC {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(UpdateNOC::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace UpdateNOC
namespace UpdateFabricLabel {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(UpdateFabricLabel::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace UpdateFabricLabel
namespace RemoveFabric {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RemoveFabric::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace RemoveFabric
namespace AddTrustedRootCertificate {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(AddTrustedRootCertificate::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace AddTrustedRootCertificate
namespace SetVIDVerificationStatement {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetVIDVerificationStatement::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace SetVIDVerificationStatement
namespace SignVIDVerificationRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SignVIDVerificationRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kAdminister);
} // namespace SignVIDVerificationRequest

} // namespace Commands
} // namespace OperationalCredentials
} // namespace Clusters
} // namespace app
} // namespace chip
