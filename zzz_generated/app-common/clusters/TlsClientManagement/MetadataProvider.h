// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TlsClientManagement (cluster code: 2050/0x802)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/TlsClientManagement/Ids.h>
#include <clusters/TlsClientManagement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::TlsClientManagement::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::TlsClientManagement::Attributes;
        switch (attributeId)
        {
        case MaxProvisioned::Id:
            return MaxProvisioned::kMetadataEntry;
        case ProvisionedEndpoints::Id:
            return ProvisionedEndpoints::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::TlsClientManagement::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::TlsClientManagement::Commands;
        switch (commandId)
        {
        case ProvisionEndpoint::Id:
            return ProvisionEndpoint::kMetadataEntry;
        case FindEndpoint::Id:
            return FindEndpoint::kMetadataEntry;
        case RemoveEndpoint::Id:
            return RemoveEndpoint::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
