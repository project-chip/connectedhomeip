// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Descriptor (cluster code: 29/0x1D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

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
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::Descriptor::Attributes;
        switch (attributeId)
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
        case EndpointUniqueID::Id:
            return EndpointUniqueID::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Descriptor::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::Descriptor::Commands;
        switch (commandId)
        {

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
