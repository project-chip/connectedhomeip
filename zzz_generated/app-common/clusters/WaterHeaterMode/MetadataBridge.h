// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WaterHeaterMode (cluster code: 158/0x9E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/WaterHeaterMode/Ids.h>
#include <clusters/WaterHeaterMode/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::WaterHeaterMode::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::WaterHeaterMode::Attributes;
        switch (commandId)
        {
        case SupportedModes::Id:
            return SupportedModes::kMetadataEntry;
        case CurrentMode::Id:
            return CurrentMode::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::WaterHeaterMode::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::WaterHeaterMode::Commands;
        switch (commandId)
        {
        case ChangeToMode::Id:
            return ChangeToMode::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
