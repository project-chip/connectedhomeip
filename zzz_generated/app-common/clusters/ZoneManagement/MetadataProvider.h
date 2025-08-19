// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ZoneManagement (cluster code: 1360/0x550)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

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
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ZoneManagement::Attributes;
        switch (attributeId)
        {
        case MaxUserDefinedZones::Id:
            return MaxUserDefinedZones::kMetadataEntry;
        case MaxZones::Id:
            return MaxZones::kMetadataEntry;
        case Zones::Id:
            return Zones::kMetadataEntry;
        case Triggers::Id:
            return Triggers::kMetadataEntry;
        case SensitivityMax::Id:
            return SensitivityMax::kMetadataEntry;
        case Sensitivity::Id:
            return Sensitivity::kMetadataEntry;
        case TwoDCartesianMax::Id:
            return TwoDCartesianMax::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ZoneManagement::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ZoneManagement::Commands;
        switch (commandId)
        {
        case CreateTwoDCartesianZone::Id:
            return CreateTwoDCartesianZone::kMetadataEntry;
        case UpdateTwoDCartesianZone::Id:
            return UpdateTwoDCartesianZone::kMetadataEntry;
        case RemoveZone::Id:
            return RemoveZone::kMetadataEntry;
        case CreateOrUpdateTrigger::Id:
            return CreateOrUpdateTrigger::kMetadataEntry;
        case RemoveTrigger::Id:
            return RemoveTrigger::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
