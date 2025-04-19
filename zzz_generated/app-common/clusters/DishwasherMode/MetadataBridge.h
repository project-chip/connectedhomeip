// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DishwasherMode (cluster code: 89/0x59)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/DishwasherMode/Ids.h>
#include <clusters/DishwasherMode/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::DishwasherMode::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::DishwasherMode::Attributes;
        switch (commandId)
        {
        case SupportedModes::Id:
            return SupportedModes::kMetadataEntry;
        case CurrentMode::Id:
            return CurrentMode::kMetadataEntry;
        case StartUpMode::Id:
            return StartUpMode::kMetadataEntry;
        case OnMode::Id:
            return OnMode::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::DishwasherMode::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::DishwasherMode::Commands;
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
