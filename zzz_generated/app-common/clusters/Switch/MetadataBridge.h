// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Switch (cluster code: 59/0x3B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/Switch/Ids.h>
#include <clusters/Switch/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::Switch::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::Switch::Attributes;
        switch (commandId)
        {
        case NumberOfPositions::Id:
            return NumberOfPositions::kMetadataEntry;
        case CurrentPosition::Id:
            return CurrentPosition::kMetadataEntry;
        case MultiPressMax::Id:
            return MultiPressMax::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Switch::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::Switch::Commands;
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
