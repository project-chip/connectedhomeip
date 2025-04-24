// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ContentLauncher (cluster code: 1290/0x50A)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ContentLauncher/Ids.h>
#include <clusters/ContentLauncher/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ContentLauncher::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::ContentLauncher::Attributes;
        switch (commandId)
        {
        case AcceptHeader::Id:
            return AcceptHeader::kMetadataEntry;
        case SupportedStreamingProtocols::Id:
            return SupportedStreamingProtocols::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ContentLauncher::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::ContentLauncher::Commands;
        switch (commandId)
        {
        case LaunchContent::Id:
            return LaunchContent::kMetadataEntry;
        case LaunchURL::Id:
            return LaunchURL::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
