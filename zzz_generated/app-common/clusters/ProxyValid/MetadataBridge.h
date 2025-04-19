// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ProxyValid (cluster code: 68/0x44)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ProxyValid/Ids.h>
#include <clusters/ProxyValid/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ProxyValid::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::ProxyValid::Attributes;
        switch (commandId)
        {

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ProxyValid::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::ProxyValid::Commands;
        switch (commandId)
        {

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
