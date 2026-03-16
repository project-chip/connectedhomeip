// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Switch (cluster code: 59/0x3B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

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
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::Switch::Attributes;
        switch (attributeId)
        {
        case NumberOfPositions::Id:
            return NumberOfPositions::kMetadataEntry;
        case CurrentPosition::Id:
            return CurrentPosition::kMetadataEntry;
        case MultiPressMax::Id:
            return MultiPressMax::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Switch::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::Switch::Commands;
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
