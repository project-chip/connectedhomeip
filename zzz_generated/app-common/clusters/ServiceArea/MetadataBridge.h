// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ServiceArea (cluster code: 336/0x150)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ServiceArea/Ids.h>
#include <clusters/ServiceArea/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ServiceArea::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::ServiceArea::Attributes;
        switch (commandId)
        {
        case SupportedAreas::Id:
            return SupportedAreas::kMetadataEntry;
        case SupportedMaps::Id:
            return SupportedMaps::kMetadataEntry;
        case SelectedAreas::Id:
            return SelectedAreas::kMetadataEntry;
        case CurrentArea::Id:
            return CurrentArea::kMetadataEntry;
        case EstimatedEndTime::Id:
            return EstimatedEndTime::kMetadataEntry;
        case Progress::Id:
            return Progress::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ServiceArea::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::ServiceArea::Commands;
        switch (commandId)
        {
        case SelectAreas::Id:
            return SelectAreas::kMetadataEntry;
        case SkipArea::Id:
            return SkipArea::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
