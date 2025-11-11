// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TlsCertificateManagement (cluster code: 2049/0x801)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/TlsCertificateManagement/Ids.h>
#include <clusters/TlsCertificateManagement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::TlsCertificateManagement::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::TlsCertificateManagement::Attributes;
        switch (attributeId)
        {
        case MaxRootCertificates::Id:
            return MaxRootCertificates::kMetadataEntry;
        case ProvisionedRootCertificates::Id:
            return ProvisionedRootCertificates::kMetadataEntry;
        case MaxClientCertificates::Id:
            return MaxClientCertificates::kMetadataEntry;
        case ProvisionedClientCertificates::Id:
            return ProvisionedClientCertificates::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::TlsCertificateManagement::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::TlsCertificateManagement::Commands;
        switch (commandId)
        {
        case ProvisionRootCertificate::Id:
            return ProvisionRootCertificate::kMetadataEntry;
        case FindRootCertificate::Id:
            return FindRootCertificate::kMetadataEntry;
        case LookupRootCertificate::Id:
            return LookupRootCertificate::kMetadataEntry;
        case RemoveRootCertificate::Id:
            return RemoveRootCertificate::kMetadataEntry;
        case ClientCSR::Id:
            return ClientCSR::kMetadataEntry;
        case ProvisionClientCertificate::Id:
            return ProvisionClientCertificate::kMetadataEntry;
        case FindClientCertificate::Id:
            return FindClientCertificate::kMetadataEntry;
        case LookupClientCertificate::Id:
            return LookupClientCertificate::kMetadataEntry;
        case RemoveClientCertificate::Id:
            return RemoveClientCertificate::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
