// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThreadNetworkDirectory (cluster code: 1107/0x453)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ThreadNetworkDirectory/Ids.h>
#include <clusters/ThreadNetworkDirectory/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ThreadNetworkDirectory::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ThreadNetworkDirectory::Attributes;
        switch (attributeId)
        {
        case PreferredExtendedPanID::Id:
            return PreferredExtendedPanID::kMetadataEntry;
        case ThreadNetworks::Id:
            return ThreadNetworks::kMetadataEntry;
        case ThreadNetworkTableSize::Id:
            return ThreadNetworkTableSize::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ThreadNetworkDirectory::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ThreadNetworkDirectory::Commands;
        switch (commandId)
        {
        case AddNetwork::Id:
            return AddNetwork::kMetadataEntry;
        case RemoveNetwork::Id:
            return RemoveNetwork::kMetadataEntry;
        case GetOperationalDataset::Id:
            return GetOperationalDataset::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
