// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PowerSourceConfiguration (cluster code: 46/0x2E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/PowerSourceConfiguration/Ids.h>
#include <clusters/PowerSourceConfiguration/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::PowerSourceConfiguration::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::PowerSourceConfiguration::Attributes;
        switch (commandId)
        {
        case Sources::Id:
            return Sources::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::PowerSourceConfiguration::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::PowerSourceConfiguration::Commands;
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
