// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster GroupKeyManagement (cluster code: 63/0x3F)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/GroupKeyManagement/Ids.h>
#include <clusters/GroupKeyManagement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::GroupKeyManagement::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::GroupKeyManagement::Attributes;
        switch (commandId)
        {
        case GroupKeyMap::Id:
            return GroupKeyMap::kMetadataEntry;
        case GroupTable::Id:
            return GroupTable::kMetadataEntry;
        case MaxGroupsPerFabric::Id:
            return MaxGroupsPerFabric::kMetadataEntry;
        case MaxGroupKeysPerFabric::Id:
            return MaxGroupKeysPerFabric::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::GroupKeyManagement::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::GroupKeyManagement::Commands;
        switch (commandId)
        {
        case KeySetWrite::Id:
            return KeySetWrite::kMetadataEntry;
        case KeySetRead::Id:
            return KeySetRead::kMetadataEntry;
        case KeySetRemove::Id:
            return KeySetRemove::kMetadataEntry;
        case KeySetReadAllIndices::Id:
            return KeySetReadAllIndices::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
