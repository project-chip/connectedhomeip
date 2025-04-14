// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster SampleMei (cluster code: 4294048800/0xFFF1FC20)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/SampleMei/Ids.h>
#include <clusters/SampleMei/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::SampleMei::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::SampleMei::Attributes;
        switch (commandId)
        {
        case FlipFlop::Id:
            return FlipFlop::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::SampleMei::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::SampleMei::Commands;
        switch (commandId)
        {
        case Ping::Id:
            return Ping::kMetadataEntry;
        case AddArguments::Id:
            return AddArguments::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
