// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster NetworkIdentityManagement (cluster code: 1104/0x450)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/NetworkIdentityManagement/Ids.h>
#include <clusters/NetworkIdentityManagement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::NetworkIdentityManagement::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::NetworkIdentityManagement::Attributes;
        switch (attributeId)
        {
        case ActiveNetworkIdentities::Id:
            return ActiveNetworkIdentities::kMetadataEntry;
        case Clients::Id:
            return Clients::kMetadataEntry;
        case ClientTableSize::Id:
            return ClientTableSize::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::NetworkIdentityManagement::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::NetworkIdentityManagement::Commands;
        switch (commandId)
        {
        case AddClient::Id:
            return AddClient::kMetadataEntry;
        case RemoveClient::Id:
            return RemoveClient::kMetadataEntry;
        case QueryIdentity::Id:
            return QueryIdentity::kMetadataEntry;
        case ImportAdminSecret::Id:
            return ImportAdminSecret::kMetadataEntry;
        case ExportAdminSecret::Id:
            return ExportAdminSecret::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
