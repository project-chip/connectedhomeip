// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Descriptor (cluster code: 29/0x1D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/Descriptor/Ids.h>
#include <clusters/Descriptor/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::Descriptor::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::Descriptor::Attributes;
        switch (commandId)
        {
        case DeviceTypeList::Id:
            return DeviceTypeList::kMetadataEntry;
        case ServerList::Id:
            return ServerList::kMetadataEntry;
        case ClientList::Id:
            return ClientList::kMetadataEntry;
        case PartsList::Id:
            return PartsList::kMetadataEntry;
        case TagList::Id:
            return TagList::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Descriptor::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::Descriptor::Commands;
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
