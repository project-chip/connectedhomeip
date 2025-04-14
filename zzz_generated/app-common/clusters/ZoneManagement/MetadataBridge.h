// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ZoneManagement (cluster code: 1360/0x550)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ZoneManagement/Ids.h>
#include <clusters/ZoneManagement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ZoneManagement::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::ZoneManagement::Attributes;
        switch (commandId)
        {
        case SupportedZoneSources::Id:
            return SupportedZoneSources::kMetadataEntry;
        case Zones::Id:
            return Zones::kMetadataEntry;
        case Triggers::Id:
            return Triggers::kMetadataEntry;
        case Sensitivity::Id:
            return Sensitivity::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ZoneManagement::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::ZoneManagement::Commands;
        switch (commandId)
        {
        case CreateTwoDCartesianZone::Id:
            return CreateTwoDCartesianZone::kMetadataEntry;
        case UpdateTwoDCartesianZone::Id:
            return UpdateTwoDCartesianZone::kMetadataEntry;
        case GetTwoDCartesianZone::Id:
            return GetTwoDCartesianZone::kMetadataEntry;
        case RemoveZone::Id:
            return RemoveZone::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
