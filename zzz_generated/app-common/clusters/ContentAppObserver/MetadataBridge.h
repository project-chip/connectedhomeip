// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ContentAppObserver (cluster code: 1296/0x510)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ContentAppObserver/Ids.h>
#include <clusters/ContentAppObserver/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ContentAppObserver::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::ContentAppObserver::Attributes;
        switch (commandId)
        {

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ContentAppObserver::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::ContentAppObserver::Commands;
        switch (commandId)
        {
        case ContentAppMessage::Id:
            return ContentAppMessage::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
