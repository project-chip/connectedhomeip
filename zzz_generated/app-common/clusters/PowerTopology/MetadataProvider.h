// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PowerTopology (cluster code: 156/0x9C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/PowerTopology/Ids.h>
#include <clusters/PowerTopology/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::PowerTopology::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::PowerTopology::Attributes;
        switch (attributeId)
        {
        case AvailableEndpoints::Id:
            return AvailableEndpoints::kMetadataEntry;
        case ActiveEndpoints::Id:
            return ActiveEndpoints::kMetadataEntry;
        default:
            return AttributeEntry({}, {}, std::nullopt, std::nullopt);
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::PowerTopology::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::PowerTopology::Commands;
        switch (commandId)
        {

        default:
            return AcceptedCommandEntry();
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
