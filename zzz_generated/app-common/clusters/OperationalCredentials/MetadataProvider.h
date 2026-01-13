// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OperationalCredentials (cluster code: 62/0x3E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/OperationalCredentials/Ids.h>
#include <clusters/OperationalCredentials/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::OperationalCredentials::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::OperationalCredentials::Attributes;
        switch (attributeId)
        {
        case NOCs::Id:
            return NOCs::kMetadataEntry;
        case Fabrics::Id:
            return Fabrics::kMetadataEntry;
        case SupportedFabrics::Id:
            return SupportedFabrics::kMetadataEntry;
        case CommissionedFabrics::Id:
            return CommissionedFabrics::kMetadataEntry;
        case TrustedRootCertificates::Id:
            return TrustedRootCertificates::kMetadataEntry;
        case CurrentFabricIndex::Id:
            return CurrentFabricIndex::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::OperationalCredentials::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::OperationalCredentials::Commands;
        switch (commandId)
        {
        case AttestationRequest::Id:
            return AttestationRequest::kMetadataEntry;
        case CertificateChainRequest::Id:
            return CertificateChainRequest::kMetadataEntry;
        case CSRRequest::Id:
            return CSRRequest::kMetadataEntry;
        case AddNOC::Id:
            return AddNOC::kMetadataEntry;
        case UpdateNOC::Id:
            return UpdateNOC::kMetadataEntry;
        case UpdateFabricLabel::Id:
            return UpdateFabricLabel::kMetadataEntry;
        case RemoveFabric::Id:
            return RemoveFabric::kMetadataEntry;
        case AddTrustedRootCertificate::Id:
            return AddTrustedRootCertificate::kMetadataEntry;
        case SetVIDVerificationStatement::Id:
            return SetVIDVerificationStatement::kMetadataEntry;
        case SignVIDVerificationRequest::Id:
            return SignVIDVerificationRequest::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
