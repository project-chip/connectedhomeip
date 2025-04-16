// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalGridConditions (cluster code: 160/0xA0)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ElectricalGridConditions/Ids.h>
#include <clusters/ElectricalGridConditions/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ElectricalGridConditions::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::ElectricalGridConditions::Attributes;
        switch (commandId)
        {
        case LocalGenerationAvailable::Id:
            return LocalGenerationAvailable::kMetadataEntry;
        case CurrentConditions::Id:
            return CurrentConditions::kMetadataEntry;
        case ForecastConditions::Id:
            return ForecastConditions::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ElectricalGridConditions::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::ElectricalGridConditions::Commands;
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
